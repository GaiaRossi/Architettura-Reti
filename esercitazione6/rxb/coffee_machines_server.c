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

int autorizza(const char *username, const char *password){
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

    /* accetto richieste */
    int ns, pid;
    for(;;){
        if((ns = accept(sd, NULL, NULL)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pid = fork();
        if(pid < 0){
            perror("creazione figlio");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            /* codice figlio */
            close(sd);

            /* disinstallazione gestore sigchld */
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                    perror("sigaction");
                    exit(EXIT_FAILURE);
            }

            /* preparazione buffers */
            char username[DIMENSIONE_BUFFER];
            char password[DIMENSIONE_BUFFER];
            char categoria[DIMENSIONE_BUFFER];

            rxb_t rxb;
            rxb_init(&rxb, DIMENSIONE_BUFFER);

            size_t username_len, password_len, categoria_len;
            
            /* leggo username */
            memset(username, 0, sizeof(username));
            username_len = sizeof(username) - 1;
            
            if(rxb_readline(&rxb, ns, username, &username_len) < 0){
                /* client che ha chiuso la connessione */
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            /* leggo password */
            memset(password, 0, sizeof(password));
            password_len = sizeof(password) - 1;

            if(rxb_readline(&rxb, ns, password, &password_len) < 0){
                /* client che ha chiuso la connessione */
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            /* autentico */
            if(autorizza(username, password) == 1){
                /* client autenticato */
                /* invio ack con ok */
                char *ok = "OK\n";
                write_all(ns, ok, strlen(ok));

                /* leggo categorie */
                for(;;){
                    memset(categoria, 0, sizeof(categoria));
                    categoria_len = sizeof(categoria) - 1;

                    if(rxb_readline(&rxb, ns, categoria, &categoria_len) < 0){
                        /* client che ha chiuso la connessione */
                        rxb_destroy(&rxb);
                        exit(EXIT_FAILURE);
                    }

                    /* preparo nome file */
                    strcat(categoria, ".txt");

                    /* sort -rn categoria | head -n 10 | cut -f1,3,4 -d' ' */
                    /* creo pipe tra sort e head */
                    int pipe_sort_head[2];

                    if(pipe(pipe_sort_head) < 0){
                        perror("pipe sort -> head");
                        exit(EXIT_FAILURE);
                    }

                    /* creo nipoti */
                    int pid_sort, pid_head, pid_cut;
                    pid_sort = fork();
                    if(pid_sort < 0){
                        perror("pid sort");
                        exit(EXIT_FAILURE);
                    }
                    else if(pid_sort == 0){
                        /* codice sort */
                        close(ns);
                        close(pipe_sort_head[0]);

                        /* redirezione */
                        close(1);
                        if(dup(pipe_sort_head[1]) < 0){
                            perror("dup sort");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_sort_head[1]);

                        execlp("sort", "sort", "-rn", categoria, (char *)0);
                        exit(EXIT_FAILURE);
                    }
                    /* codice figlio */
                    /* creazione pipe head cut */
                    int pipe_head_cut[2];

                    if(pipe(pipe_head_cut) < 0){
                        perror("pipe head -> cut");
                        exit(EXIT_FAILURE);
                    }

                    pid_head = fork();
                    if(pid_head < 0){
                        perror("pid head");
                        exit(EXIT_FAILURE);
                    }
                    else if(pid_head == 0){
                        /* codice head */
                        close(ns);
                        close(pipe_head_cut[0]);
                        close(pipe_sort_head[1]);

                        close(0);
                        if(dup(pipe_sort_head[0]) < 0){
                            perror("dup head");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_sort_head[0]);

                        /* redirezione */
                        close(1);
                        if(dup(pipe_head_cut[1]) < 0){
                            perror("dup head");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_head_cut[1]);

                        execlp("head", "head", "-n", "10", (char *)0);
                        exit(EXIT_FAILURE);
                    }
                    /* codice figlio */
                    close(pipe_sort_head[0]);
                    close(pipe_sort_head[1]);

                    pid_cut = fork();
                    if(pid_cut < 0){
                        perror("pid cut");
                        exit(EXIT_FAILURE);
                    }
                    else if(pid_cut == 0){
                        /* codice cut */
                        close(pipe_head_cut[1]);

                        close(0);
                        if(dup(pipe_head_cut[0]) < 0){
                            perror("dup cut");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_head_cut[0]);

                        /* redirezione */
                        close(1);
                        if(dup(ns) < 0){
                            perror("dup cut");
                            exit(EXIT_FAILURE);
                        }
                        close(ns);

                        execlp("cut", "cut", "-f1,3,4", (char *)0);
                        exit(EXIT_FAILURE);
                    }
                    /* codice figlio */
                    close(pipe_head_cut[0]);
                    close(pipe_head_cut[1]);

                    /* attendo nipoti */
                    waitpid(pid_sort, &status, 0);
                    waitpid(pid_head, &status, 0);
                    waitpid(pid_cut, &status, 0);

                    /* dico a client che ho inviato tutti i dati */
                    char *fine_richiesta = "fine_richiesta\n";
                    write_all(ns, fine_richiesta, strlen(fine_richiesta));
                }
                rxb_destroy(&rxb);
                close(ns);
                exit(EXIT_SUCCESS);
            }
            else{
                /* client non autenticato */
                char *no = "NO\n";
                write_all(ns, no, strlen(no));
                close(ns);
                exit(EXIT_FAILURE);
            }
        }
        /* codice padre */
        close(ns);
    }
}