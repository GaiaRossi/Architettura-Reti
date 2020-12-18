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

    int pid, nuovo_sd;
    for(;;){
        if((nuovo_sd = accept(sd, NULL, NULL)) < 0){
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
            char buffer[DIMENSIONE_BUFFER];
            size_t buffer_len;
            char netOut[DIMENSIONE_BUFFER];
            rxb_t rxb;
            
            rxb_init(&rxb, DIMENSIONE_BUFFER);

            for(;;){
                memset(buffer, 0, sizeof(buffer));
                buffer_len = sizeof(buffer) - 1;

                if(rxb_readline(&rxb, nuovo_sd, buffer, &buffer_len) < 0){
                    break;
                }

                memset(netOut, 0, sizeof(netOut));
                snprintf(netOut, sizeof(netOut) - 1, "La dimensione della stringa e': %ld\n", strlen(buffer));

                write_all(nuovo_sd, netOut, strlen(netOut));

                char *fine_richiesta =  "fine\n";
                write_all(nuovo_sd, fine_richiesta, strlen(fine_richiesta));
            }
            rxb_destroy(&rxb);
            close(nuovo_sd);
            exit(EXIT_SUCCESS);
        }
        /* codice padre */
        close(nuovo_sd);
    }

}