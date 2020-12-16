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

int main(int argc, char **argv){
    /* bollettino_neve server porta */
    /* controllo argomenti */
    if(argc != 3){
        fprintf(stderr, "Uso: ./client server porta\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err;
    if((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0){
        fprintf(stderr, "Errore getaddrinfo\n");
        exit(EXIT_FAILURE);
    }

    /* connessione con fallback */
    struct addrinfo *ptr;
    int sd;
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next){
        if((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) > 0){
            if(connect(sd, ptr->ai_addr, ptr->ai_addrlen) < 0){
                close(sd);
                continue;
            }
            else{
                break;
            }
        }
    }

    if(ptr == NULL){
        fprintf(stderr, "Nessuna connesione\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    /* preparazione buffers */
    char netOut[DIMENSIONE_BUFFER];
    char netIn[DIMENSIONE_BUFFER];

    memset(netOut, 0, sizeof(netOut));
    memset(netIn, 0, sizeof(netIn));

    rxb_t rxb;

    rxb_init(&rxb, DIMENSIONE_BUFFER);

    for(;;){
        printf("Inserisci la stringa:\n");
        fgets(netOut, sizeof(netOut) - 1, stdin);

        if(strcmp(netOut, "fine\n") == 0){
            break;
        }

        write_all(sd, netOut, strlen(netOut));

        /* lettura risposta */
        size_t netIn_len;
        for(;;){
            netIn_len = sizeof(netIn) - 1;
            memset(netIn, 0, sizeof(netIn));

            rxb_readline(&rxb, sd, netIn, &netIn_len);
            if(strcmp(netIn, "fine") == 0){
                break;
            }
            puts(netIn);
        }
    }

    rxb_destroy(&rxb);
    close(sd);
}