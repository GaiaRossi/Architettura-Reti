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

//lista_articoli   server   porta
#define DIMENSIONE_BUFFER 4096

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
    char email[DIMENSIONE_BUFFER];
    char password[DIMENSIONE_BUFFER];
    char rivista[DIMENSIONE_BUFFER];

    memset(netOut, 0, sizeof(netOut));
    memset(netIn, 0, sizeof(netIn));
    memset(email, 0, sizeof(email));
    memset(password, 0, sizeof(password));
    memset(rivista, 0, sizeof(rivista));
    
    rxb_t rxb;
    rxb_init(&rxb, DIMENSIONE_BUFFER);

    /* lettura input */
    for(;;){
        printf("Inserisci email:\n");
        fgets(email, sizeof(email) - 1, stdin);

        if(strcmp(email, "fine\n") == 0){
            break;
        }

        printf("Inserisci la password:\n");
        fgets(password, sizeof(password) - 1, stdin);

        if(strcmp(password, "fine\n") == 0){
            break;
        }

        printf("Inserisci la rivista di interesse:\n");
        fgets(rivista, sizeof(rivista) - 1, stdin);

        if(strcmp(rivista, "fine\n") == 0){
            break;
        }

        strcpy(netOut, email);
        strcat(netOut, password);
        strcat(netOut, rivista);

        /* invio al server */
        write_all(sd, netOut, strlen(netOut));

        /* Leggo la risposta del server e la stampo a video */
        char response[DIMENSIONE_BUFFER];
        memset(response, 0, sizeof(response));
        size_t response_len;

        while(strcmp(response, "finerichiesta") != 0){
            response_len = sizeof(response) - 1;

            memset(response, 0, sizeof(response));

            /* Ricezione risultato */
            if (rxb_readline(&rxb, sd, response, &response_len) < 0) {
                rxb_destroy(&rxb);
                fprintf(stderr, "Connessione chiusa dal server!\n");
                exit(EXIT_FAILURE);
            }

            /* Stampo riga letta da Server */
            if(strcmp(response, "finerichiesta") != 0){
                puts(response);            
            }
        }
    }

    printf("Richiesta finita\n");
    rxb_destroy(&rxb);
    close(sd);
}