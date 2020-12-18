#define _POSIX_C_SOURCE	200809L
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <unistr.h>

int main(int argc, char** argv){

    //rgrep hostname porta stringa nomefile
    //controllo argomenti
    if(argc != 5){
        fprintf(stderr, "Uso: ./client hostname porta stringa nomefile\n");
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int err;

    if((err = getaddrinfo(argv[1], argv[2], &hints, &res)) < 0){
        fprintf(stderr, "Errore in getaddrinfo\n");
        exit(EXIT_FAILURE);
    }

    int sd;

    /*connessione con fallback*/
    struct addrinfo *ptr;
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next){
        /*provo a connettermi con qualcuno*/
        if((sd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) > 0){
            if(connect(sd, ptr->ai_addr, ptr->ai_addrlen) == 0){
                break;
            }
            else{
                close(sd);
            }
        }
    }

    if(ptr == NULL){
        fprintf(stderr, "Nessuna connessione\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    /*codice esercizio*/
    //invio nome file
    write(sd, argv[4], strlen(argv[4]));

    //attendo risposta
    char risposta[2];
    memset(risposta, 0, sizeof(risposta));
    read(sd, risposta, sizeof(risposta) - 1);

    //controllo risposta
    printf("risposta: %s\n", risposta);
    if(strcmp(risposta, "Y") == 0){
        printf("Il file esiste\n");
    }
    else{
        printf("Il file non esiste\n");
        close(sd);
        exit(EXIT_FAILURE);
    }

    //invio la stringa da cercare
    write(sd, argv[3], strlen(argv[3]));

    //leggo risposta
    char buffer[2048];
    memset(buffer, 0, sizeof(buffer));
    read(sd, buffer, sizeof(buffer) - 1);

    printf("%s\n", buffer);

    close(sd);
}