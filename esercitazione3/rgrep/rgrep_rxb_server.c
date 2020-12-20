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

    int pid, ns;
    for(;;){
        if((ns = accept(sd, NULL, NULL)) < 0){
            perror("accept");
            close(sd);
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
            char nome_file[DIMENSIONE_BUFFER];
            char stringa[DIMENSIONE_BUFFER];
            char buffer[DIMENSIONE_BUFFER];
            size_t buffer_len;

            memset(nome_file, 0, sizeof(nome_file));
            memset(stringa, 0, sizeof(stringa));

            rxb_t rxb;
            rxb_init(&rxb, DIMENSIONE_BUFFER);

            /* ricevo stringa con nome file e stringa */
            memset(buffer, 0, sizeof(buffer));
            buffer_len = sizeof(buffer) - 1;
            if(rxb_readline(&rxb, ns, buffer, &buffer_len) < 0){
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            /* salvo nome file */
            snprintf(nome_file, sizeof(nome_file), "%s", buffer);

            int fd;
            if((fd = open(nome_file, O_RDONLY)) < 0){
                /* file non esiste */
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }
            close(fd);

            /* salvo stringa */
            memset(buffer, 0, sizeof(buffer));
            buffer_len = sizeof(buffer) - 1;
            if(rxb_readline(&rxb, ns, buffer, &buffer_len) < 0){
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            strcpy(stringa, buffer);

            /* creo nipote */
            int pidn;
            if((pidn = fork()) < 0){
                perror("fork nipote");
                exit(EXIT_FAILURE);
            }
            else if(pidn == 0){
                /* codice nipote */
                close(1);
                dup(ns);
                close(ns);

                execlp("grep", "grep", stringa, nome_file, (char *)0);
                exit(EXIT_FAILURE);
            }

            wait(&status);

            char *fine_richiesta = "fine_richiesta\n";
            write_all(ns, fine_richiesta, strlen(fine_richiesta));

            /* non mi server piu il buffer di ricezione */
            rxb_destroy(&rxb);

            close(ns);
            exit(EXIT_SUCCESS);
        }
        /* codice padre */
        close(ns);
    }
}