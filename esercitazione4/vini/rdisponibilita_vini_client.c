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
    //./client localhost porta
    //controllo argomenti
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

    /* lettura input utente */
    char input[2048], ack[4], risposta[2048];
    memset(input, 0, sizeof(input));
    
    //guardare uso di fgets
    scanf("%s", input);

    while(strcmp(input, "fine") != 0){
        //invio vino al server
        write(sd, input, strlen(input));

        //attendo ack
        memset(ack, 0, sizeof(ack));
        read(sd, ack, sizeof(ack) - 1);

        //leggo e invio annata
        memset(input, 0, sizeof(input));
        scanf("%s", input);
        write(sd, input, strlen(input));

        //attendo risposta
        memset(risposta, 0, sizeof(risposta));
        read(sd, risposta, sizeof(risposta) - 1);
        while (strcmp(risposta, "fine") != 0){
            write(STDOUT_FILENO, risposta, strlen(risposta));
            memset(risposta, 0, sizeof(risposta));
            read(sd, risposta, sizeof(risposta) - 1);
        }

        //preparo per prossimo input
        write(STDOUT_FILENO, "Inserisci nuovo elemento\n", strlen("Inserisci nuovo elemento\n"));
        memset(input, 0, sizeof(input));
        scanf("%s", input);        
    }

    //avviso il server di chiudere
    write(sd, input, strlen(input));
    close(sd);
}