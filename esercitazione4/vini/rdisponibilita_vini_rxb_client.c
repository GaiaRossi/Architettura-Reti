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
    /* verifica_disponibilità_vini server porta */
    /* controllo argomenti */
    if(argc != 3){
        fprintf(stderr, "Uso: ./client_rxb server porta\n");
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
    char tastiera[DIMENSIONE_BUFFER];
    char netOut[DIMENSIONE_BUFFER];
    char netIn[DIMENSIONE_BUFFER];

    rxb_t rxb;
    rxb_init(&rxb, DIMENSIONE_BUFFER);

    size_t netIn_len;

    /* leggo input, invio a server, attendo risposta */
    for(;;){
        /* leggo input */
        memset(tastiera, 0, sizeof(tastiera));
        memset(netOut, 0, sizeof(netOut));

        printf("Inserisci il nome del vino:\n");
        if(fgets(tastiera, sizeof(tastiera), stdin) == NULL){
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if(strcmp(tastiera, "fine\n") == 0){
            break;
        }

        strcpy(netOut, tastiera);

        printf("Inserisci l'annata del vino:\n");
        if(fgets(tastiera, sizeof(tastiera), stdin) == NULL){
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        if(strcmp(tastiera, "fine\n") == 0){
            break;
        }

        strcat(netOut, tastiera);

        /* invio a server */
        if(write_all(sd, netOut, strlen(netOut)) < 0){
            perror("write all");
            exit(EXIT_FAILURE);
        }

        /* leggo risposta */
        for(;;){
            netIn_len = sizeof(netIn_len) - 1;
            memset(netIn, 0, sizeof(netIn));

            if(rxb_readline(&rxb, sd, netIn, &netIn_len) < 0){
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            if(strcmp(netIn, "fine_richiesta") == 0){
                break;
            }

            puts(netIn);
        }
    }
    rxb_destroy(&rxb);
    close(sd);
    exit(EXIT_SUCCESS);
}