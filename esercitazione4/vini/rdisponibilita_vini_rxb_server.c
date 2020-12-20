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

#define DIMENSIONE_BUFFER 2048

int status;

void child_handler(int signo){
    while(waitpid(-1, &status, WNOHANG) > 0);
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

    int on = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt");
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

    int ns, pid;
    for(;;){
        if((ns = accept(sd, NULL, NULL)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        if((pid = fork()) < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            /* codice figlio */
            close(sd);

            /* Disabilito gestore SIGCHLD */
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                    perror("sigaction");
                    exit(EXIT_FAILURE);
            }

            /* preparazione buffers */
            char nome_vino[DIMENSIONE_BUFFER];
            char annata[DIMENSIONE_BUFFER];
            size_t nome_vino_len, annata_len;

            rxb_t rxb;
            rxb_init(&rxb, DIMENSIONE_BUFFER);

            for(;;){
                /* leggo nome vino */
                memset(nome_vino, 0, sizeof(nome_vino));
                memset(annata, 0, sizeof(annata));

                nome_vino_len = sizeof(nome_vino) - 1;
                annata_len = sizeof(annata) - 1;

                if(rxb_readline(&rxb, ns, nome_vino, &nome_vino_len) < 0){
                    break;
                }

                /* leggo annata */
                if(rxb_readline(&rxb, ns, annata, &annata_len) < 0){
                    break;
                }


                char debug[3 * DIMENSIONE_BUFFER];
                snprintf(debug, sizeof(debug), "letti: %s, %s\n", nome_vino, annata);
                write_all(STDOUT_FILENO, debug, strlen(debug));


                /* creo pipe e genero nipoti */
                int pipefd[2];

                if(pipe(pipefd) < 0){
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                int pidn1, pidn2;
                if((pidn1 = fork()) < 0){
                    perror("fork grep 1");
                    exit(EXIT_FAILURE);
                }
                else if(pidn1 == 0){
                    /* codice grep 1 */
                    close(pipefd[0]);
                    close(ns);

                    close(1);
                    if(dup(pipefd[1]) < 0){
                        perror("dup 1");
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd[1]);

                    execlp("grep", "grep", nome_vino, "vini.txt", (char *)0);
                    exit(EXIT_FAILURE);
                }

                if((pidn2 = fork()) < 0){
                    perror("fork grep 2");
                    exit(EXIT_FAILURE);
                }
                else if(pidn2 == 0){
                    /* codice grep 2 */
                    close(0);
                    if(dup(pipefd[0]) < 0){
                        perror("dup 2");
                        exit(EXIT_FAILURE);
                    }
                    close(pipefd[0]);

                    close(1);
                    if(dup(ns) < 0){
                        perror("dup 2");
                        exit(EXIT_FAILURE);
                    }
                    close(ns);

                    execlp("grep", "grep", annata, (char *)0);
                    exit(EXIT_FAILURE);
                }
                close(pipefd[0]);
                close(pipefd[1]);

                waitpid(pidn1, &status, 0);
                waitpid(pidn2, &status, 0);

                char *fine_richiesta = "fine_richiesta\n";
                write_all(ns, fine_richiesta, strlen(fine_richiesta));
            }

            rxb_destroy(&rxb);
            close(ns);
            exit(EXIT_SUCCESS);
        }
        /* codice padre */
        close(ns);
    }
}