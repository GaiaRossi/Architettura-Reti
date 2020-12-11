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

    /* lettura input */
    char input[2048];
    char risposta[2048];
    memset(input, 0, sizeof(input));
    //lettura categoria
    scanf("%s", input);

    while(strcmp(input, "fine") != 0){
        //invio a server
        write(sd, input, strlen(input) + 1);
        //attendo risposta
        memset(risposta, 0, sizeof(risposta));
        read(sd, risposta, sizeof(risposta) - 1);
        while(strcmp(risposta, "fine\n") != 0){
            //stampo risposta
            write(STDOUT_FILENO, risposta, strlen(risposta));
            memset(risposta, 0, sizeof(risposta));
            read(sd, risposta, sizeof(risposta) - 1);
        }
        //debug
        char debug[1024];
        memset(debug, 0, sizeof(debug));
        snprintf(debug, strlen("Inserisci nuovo elemento\n") + 1, "Inserisci nuovo elemento\n");
        write(STDOUT_FILENO, debug, strlen(debug));


        //preparo per il prossimo input
        memset(input, 0, sizeof(input));
        scanf("%s", input);
    }

    //invio fine a server
    write(sd, input, strlen(input) + 1);

    close(sd);
}
