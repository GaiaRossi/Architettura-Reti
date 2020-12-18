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
            close(ns);
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

            /* lettura lunghezza opzioni */
            uint8_t len[2];
            char opzioni[DIMENSIONE_BUFFER];

            if(read_all(ns, len, 2) < 0){
                perror("read all");
                exit(EXIT_FAILURE);
            }

            int opzioni_len = (len[0] << 8) | len[1];

            /* lettura opzioni */
            if(read_all(ns, opzioni, opzioni_len) < 0){
                perror("read all");
                exit(EXIT_FAILURE);
            }

            int pidn, pipefd[2];

            if(pipe(pipefd) < 0){
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            if((pidn = fork()) < 0){
                perror("fork nipote");
                exit(EXIT_FAILURE);
            }
            else if(pidn == 0){
                /* codice nipote */
                close(ns);
                close(pipefd[0]);

                close(1);
                dup(pipefd[1]);
                close(pipefd[1]);
                  
                execlp("ps", "ps", opzioni, (char *)0);
                exit(EXIT_FAILURE);
            }
            
            close(pipefd[1]);

            wait(&status);

            char debug[DIMENSIONE_BUFFER];
            memset(debug, 0, sizeof(debug));
            snprintf(debug, sizeof(debug), "figlio\n");
            write_all(1, debug, strlen(debug));

            int nread, response_len;
            char response[65536];
            int read_so_far = 0;

            while ((nread = read(pipefd[0], response + read_so_far, sizeof(response) - read_so_far)) > 0) {
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

            close(pipefd[0]);
            close(ns);
            exit(EXIT_SUCCESS);   
        }
        /* codice padre */
        close(ns);
    }
}