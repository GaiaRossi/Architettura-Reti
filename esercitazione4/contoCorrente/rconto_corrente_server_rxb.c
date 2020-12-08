#define _POSIX_C_SOURCE	200809L
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistr.h>
#include "utils.h"
#include "rxb.h"

#define MAX_REQUEST_SIZE 1024*64

int status;

void child_handler(int signo){
    while(waitpid(-1, &status, WNOHANG) > 0){
        continue;
    }
}

int main(int argc, char** argv){
    //controllo argomenti
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
        exit(EXIT_FAILURE);
    }

    //installo segnali
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = child_handler;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("Errore sigaction");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0){
        fprintf(stderr, "Errore getaddrinfo\n");
        exit(EXIT_FAILURE);
    }

    int sd;
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    if(bind(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore bind");
        close(sd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if(listen(sd, SOMAXCONN) < 0){
        perror("Errore listen");
        close(sd);
        exit(EXIT_FAILURE);
    }

    int nuovo_sd, pid;
    for(;;){
        //controllo valore di ritorno accept
        if((nuovo_sd = accept(sd, NULL, NULL)) < 0){
            perror("Errore accept");
            exit(EXIT_FAILURE);
        }
        pid = fork();

        if(pid < 0){
            perror("Errore generazione figlio");
            close(sd);
            close(nuovo_sd);
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            //codice figlio
            close(sd);
            rxb_t rxb;

            //disabilito gestore dei segnali
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if(sigaction(SIGCHLD, &sa, NULL) == -1){
                perror("Errore sigaction figlio");
                exit(EXIT_FAILURE);
            }

            //leggo spese richieste
            uint8_t categoria[MAX_REQUEST_SIZE];
            size_t categoria_len = sizeof(categoria) - 1;
            //inizializzo il buffer di ricezione
            rxb_init(&rxb, MAX_REQUEST_SIZE);
            memset(categoria, 0, sizeof(categoria));
            
            while(rxb_readline(&rxb, nuovo_sd, categoria, &categoria_len) != -1){
                //controllo validita stringa ricevuta
                if(u8_check(categoria, categoria_len) != NULL){
                    fprintf(stderr, "Stringa non in UTF-8\n");
                    close(nuovo_sd);
                    exit(EXIT_FAILURE);
                }
                
                //pipe tra nipoti
                int pipefd[2];
                if(pipe(pipefd) < 0){
                    perror("Errore creazione pipe");
                    close(nuovo_sd);
                    exit(EXIT_FAILURE);
                }

                int pid_nipote;
                pid_nipote = fork();
                if(pid_nipote < 0){
                    perror("Errore generazione nipote grep");
                    close(nuovo_sd);
                    exit(EXIT_FAILURE);
                }
                else if(pid_nipote == 0){
                    //codice nipote grep
                    close(nuovo_sd);
                    close(pipefd[0]);
                    //redirezione
                    close(1);
                    dup(pipefd[1]);
                    close(pipefd[1]);

                    execlp("grep", "grep", categoria, "conto_corrente.txt", (char *)0);
                    exit(EXIT_FAILURE);
                }

                pid_nipote = fork();
                if(pid_nipote < 0){
                    perror("Errore generazione nipote sort");
                    close(nuovo_sd);
                    exit(EXIT_FAILURE);
                }
                else if(pid_nipote == 0){
                    //codice nipote sort
                    close(pipefd[1]);
                    //redirezione
                    close(0);
                    dup(pipefd[0]);
                    close(pipefd[0]);

                    close(1);
                    dup(nuovo_sd);
                    close(nuovo_sd);

                    execlp("sort", "sort", "-rn", (char *)0);
                    exit(EXIT_FAILURE);
                }

                //codice figlio
                close(pipefd[0]);
                close(pipefd[1]);

                //attendo i figli
                wait(&status);
                wait(&status);

                rxb_destroy(&rxb);

                //avviso che ho finito
                char *fine = "fine\n";
                write_all(nuovo_sd, fine, strlen(fine));
            }
            //debug
            char *debug = "uscito da while\n";
            write(STDOUT_FILENO, debug, strlen(debug));
            exit(EXIT_SUCCESS);
        }
        //codice padre
        close(nuovo_sd);
    }
   
}
