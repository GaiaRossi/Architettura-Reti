#define _POSIX_C_SOURCE	200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <unistr.h>
#include <signal.h>
#include <wait.h>

int status;

int main(int argc, char** argv){
    //controllo argomenti
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
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
        nuovo_sd = accept(sd, NULL, NULL);
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

            //leggo vino
            char vino[2048];
            memset(vino, 0, sizeof(vino));
            read(nuovo_sd, vino, sizeof(vino) - 1);
            while(strcmp(vino, "fine") != 0){
                //invio ack
                char ack[4] = "ack\0";
                write(nuovo_sd, ack, strlen(ack));
                //leggo annata
                char annata[1024];
                memset(annata, 0, sizeof(annata));
                read(nuovo_sd, annata, sizeof(annata) - 1);
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
                    perror("Errore generazione nipote grep1");
                    close(nuovo_sd);
                    exit(EXIT_FAILURE);
                }
                else if(pid_nipote == 0){
                    //codice nipote grep1
                    close(nuovo_sd);
                    close(pipefd[0]);
                    //ridirezione
                    close(1);
                    dup(pipefd[1]);
                    close(pipefd[1]);

                    execlp("grep", "grep", vino, "vini.txt", (char *)0);
                    exit(EXIT_FAILURE);
                }

                pid_nipote = fork();
                if(pid_nipote < 0){
                    perror("Errore generazione nipote grep2");
                    close(nuovo_sd);
                    exit(EXIT_FAILURE);
                }
                else if(pid_nipote == 0){
                    //codice nipote grep2
                    close(pipefd[1]);
                    //ridirezione
                    close(0);
                    dup(pipefd[0]);
                    close(pipefd[0]);

                    close(1);
                    dup(nuovo_sd);
                    close(nuovo_sd);

                    execlp("grep", "grep", annata, (char *)0);
                    exit(EXIT_FAILURE);
                }

                //codice figlio
                close(pipefd[0]);
                close(pipefd[1]);

                //attendo i figli
                wait(&status);
                wait(&status);

                //avviso che ho finito
                char fine[1024];
                memset(fine, 0, sizeof(fine));
                snprintf(fine, strlen("fine") + 1, "fine");
                write(nuovo_sd, fine, strlen(fine));

                //leggo il prossimo vino
                memset(vino, 0, sizeof(vino));
                read(nuovo_sd, vino, sizeof(vino) - 1);
            }
            //debug
            char debug[1024];
            memset(debug, 0, sizeof(debug));
            snprintf(debug, strlen("Uscito da while\n") + 1, "Uscito da while\n");
            write(STDOUT_FILENO, debug, strlen(debug));
            exit(EXIT_SUCCESS);
        }
        //codice padre
        close(nuovo_sd);
    }
 

}
