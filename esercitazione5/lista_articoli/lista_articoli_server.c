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

#define DIMENSIONE_BUFFER 4096

int status;

void child_handler(int signo){
    while(waitpid(-1, &status, WNOHANG) > 0);
}

int autorizza(const char *email_revisore, const char *password){
	return 1;
}

int main(int argc, char** argv){
    /* controllo argomenti */
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
        exit(EXIT_FAILURE);
    }

    /* installo segnali */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = SA_RESTART;
    sa.sa_handler = child_handler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
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

    int pid, nuovo_sd;
    for(;;){
        if((nuovo_sd = accept(sd, NULL, NULL)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if((pid = fork()) < 0){
            perror("Errore generazione figlio");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            /* codice figlio */
            close(sd);

            /* disinstallo segnali */
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }
            
            /* preparazione buffers */
            char netIn[DIMENSIONE_BUFFER];
            char email[DIMENSIONE_BUFFER];
            char password[DIMENSIONE_BUFFER];
            char rivista[DIMENSIONE_BUFFER];

            rxb_t rxb;
            rxb_init(&rxb, DIMENSIONE_BUFFER);
            
            /* ricevo richiesta */
            size_t netIn_len;
            for(;;){
                memset(netIn, 0, sizeof(netIn));
                memset(email, 0, sizeof(email));
                memset(password, 0, sizeof(password));
                memset(rivista, 0, sizeof(rivista));
                netIn_len = sizeof(netIn) - 1;
 
                if(rxb_readline(&rxb, nuovo_sd, netIn, &netIn_len) < 0){
                    break;
                }
                strcpy(email, netIn);

                memset(netIn, 0, sizeof(netIn));

                if(rxb_readline(&rxb, nuovo_sd, netIn, &netIn_len) < 0){
                    break;
                }
                strcpy(password, netIn);

                memset(netIn, 0, sizeof(netIn));

                if(rxb_readline(&rxb, nuovo_sd, netIn, &netIn_len) < 0){
                    break;
                }
                strcpy(rivista, netIn);

                if(autorizza(email, password) == 1){
                    /* revisore autenticato */

                    /* preparo pipe per nipoti */
                    int pipefd[2];

                    if(pipe(pipefd) < 0){
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }

                    int pidn;
                    if((pidn = fork()) < 0){
                        perror("Nipote grep");
                        exit(EXIT_FAILURE);
                    }
                    else if(pidn == 0){
                        /* grep */
                        close(pipefd[0]);
                        close(nuovo_sd);

                        close(1);
                        dup(pipefd[1]);
                        close(pipefd[1]);

                        execlp("grep", "grep", email, "file_riviste.txt", (char *)0);
                        exit(EXIT_FAILURE);
                    }

                    /* preparo pipe */
                    int pipefd1[2];

                    if(pipe(pipefd1) < 0){
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    }

                    if((pidn = fork()) < 0){
                        perror("Nipote grep2");
                        exit(EXIT_FAILURE);
                    }
                    else if(pidn == 0){
                        /* grep */
                        close(pipefd[1]);
                        close(0);
                        dup(pipefd[0]);
                        close(pipefd[0]);

                        close(pipefd1[0]);
                        close(1);
                        dup(pipefd1[1]);
                        close(pipefd1[1]);

                        execlp("grep", "grep", rivista, (char *)0);
                        exit(EXIT_FAILURE);
                    }
                    
                    close(pipefd[0]);
                    close(pipefd[1]);

                    if((pidn = fork()) < 0){
                        perror("Nipote sort");
                        exit(EXIT_FAILURE);
                    }
                    else if(pidn == 0){
                        /* sort */
                        close(pipefd1[1]);
                        close(0);
                        dup(pipefd1[0]);
                        close(pipefd1[0]);

                        close(1);
                        dup(nuovo_sd);
                        close(nuovo_sd);

                        execlp("sort", "sort", "-rn", (char *)0);
                        exit(EXIT_FAILURE);
                    }

                    close(pipefd1[0]);
                    close(pipefd1[1]);

                    wait(&status);
                    wait(&status);

                    char *fine_richiesta = "finerichiesta\n";
                    write_all(nuovo_sd, fine_richiesta, strlen(fine_richiesta));
                }
            }
            close(nuovo_sd);
            rxb_destroy(&rxb);
            exit(EXIT_SUCCESS);
        }
        /* codice padre */
        close(nuovo_sd);
    }
}