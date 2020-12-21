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

    /* ciclo per accettare le richieste */
    int ns, pid;
    for(;;){
        if((ns = accept(sd, NULL, NULL)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pid = fork();

        if(pid < 0){
            perror("fork");
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

            uint8_t len[2];

            int username_len, password_len, categoria_len;

            /* lettura lunghezza username */
            if(read_all(ns, len, 2) < 0){
                perror("read all");
                exit(EXIT_FAILURE);
            }

            /* calcolo lunghezza e ricezione di username */
            username_len = len[0] << 8 | len[1];

            if(username_len == 0){
                break;
            }
            
            if(read_all(ns, username, username_len) < 0){
                perror("read all");
                exit(EXIT_FAILURE);
            }
            
            /* lettura password */
            if(read_all(ns, len, 2) < 0){
                perror("read all");
                exit(EXIT_FAILURE);
            }

            /* calcolo lunghezza e ricezione di password */
            password_len = len[0] << 8 | len[1];

            if(password_len == 0){
                break;
            }
            
            if(read_all(ns, password, password_len) < 0){
                perror("read all");
                exit(EXIT_FAILURE);
            }

            /* autenticazione */
            if(autorizza(username, password) == 1){
                /* client autorizzato */

                /* invio ack */
                const char *ok = "02OK";
                if(write_all(ns, ok, 4) < 0){
                    perror("write all");
                    exit(EXIT_FAILURE);
                }

                for(;;){
                    /* lettura lunghezza categoria */
                    if(read_all(ns, len, 2) < 0){
                        perror("read all");
                        exit(EXIT_FAILURE);
                    }

                    /* calcolo lunghezza e lettura categoria */
                    categoria_len = len[0] << 8 | len[1];

                    /* controllo se connessione chiusa */
                    /* error-prone */
                    if(categoria_len == 0){
                        break;
                    }

                    memset(categoria, 0, sizeof(categoria));
                    if(read_all(ns, categoria, categoria_len) < 0){
                        perror("read all");
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

                    /* creazione pipe cut e figlio */
                    int pipe_cut_figlio[2];
                    if(pipe(pipe_cut_figlio) < 0){
                        perror("pipe cut -> figlio");
                        exit(EXIT_FAILURE);
                    }

                    pid_cut = fork();
                    if(pid_cut < 0){
                        perror("pid cut");
                        exit(EXIT_FAILURE);
                    }
                    else if(pid_cut == 0){
                        /* codice cut */
                        close(pipe_head_cut[1]);
                        close(ns);
                        close(pipe_cut_figlio[0]);

                        close(0);
                        if(dup(pipe_head_cut[0]) < 0){
                            perror("dup cut");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_head_cut[0]);

                        /* redirezione */
                        close(1);
                        if(dup(pipe_cut_figlio[1]) < 0){
                            perror("dup cut");
                            exit(EXIT_FAILURE);
                        }
                        close(pipe_cut_figlio[1]);

                        execlp("cut", "cut", "-f1,3,4", (char *)0);
                        exit(EXIT_FAILURE);
                    }
                    /* codice figlio */
                    close(pipe_head_cut[0]);
                    close(pipe_head_cut[1]);

                    close(pipe_cut_figlio[1]);

                    char response[65536];
                    int response_len;
                    int read_so_far = 0;
                    int nread;

                    while ((nread = read(pipe_cut_figlio[0], response + read_so_far, 
                                sizeof(response) - read_so_far)) > 0) {
                        read_so_far += nread;
                    }

                    if (nread < 0) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }

                    if (read_so_far > 65535) {
                        fprintf(stderr, "Troppi dati\n");
                        exit(EXIT_FAILURE);
                    }

                    response_len = read_so_far;

                    len[0] = (response_len & 0xFF00) >> 8;
                    len[1] = (response_len & 0x00FF);

                    if (write_all(ns, len, 2) < 0) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }

                    if (write_all(ns, response, response_len) < 0) {
                        perror("write");
                        exit(EXIT_FAILURE);
                    }

                    /* attesa di tutti i nipoti */
                    waitpid(pid_sort, &status, 0);
                    waitpid(pid_head, &status, 0);
                    waitpid(pid_cut, &status, 0);
                }
                close(ns);
                exit(EXIT_SUCCESS);
            }
            else{
                /* client non autorizzato */
                /* invio NO */
                const char *no = "02NO";

                if(write_all(ns, no, 4) < 0){
                    perror("write all");
                    exit(EXIT_FAILURE);
                }

                close(ns);
                exit(EXIT_FAILURE);
            }

        }
        /* codice padre */
        close(ns);
    }
}