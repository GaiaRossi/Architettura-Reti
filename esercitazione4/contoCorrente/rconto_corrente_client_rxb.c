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

#define MAX_REQUEST_SIZE 1024*64

int main(int argc, char** argv){
    //./client localhost porta
    //controllo argomenti
    if(argc != 3){
        fprintf(stderr, "Uso: ./client server porta\n");
        exit(EXIT_FAILURE);
    }
    
    rxb_t rxb;
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

    rxb_init(&rxb, MAX_REQUEST_SIZE);

    /* lettura input */
    char input[2048];
    char risposta[2048];
    memset(input, 0, sizeof(input));
    //lettura categoria
    if(fgets(input, sizeof(input), stdin) == NULL){
        perror("Errore fgets");
        close(sd);
        exit(EXIT_FAILURE);
    }

    while(strcmp(input, "fine") != 0){
        //invio a server
        if(write_all(sd, input, strlen(input)) < 0){
            perror("Errore write all");
            close(sd);
            exit(EXIT_FAILURE);
        }
        //attendo risposta
        size_t risposta_len = sizeof(risposta) - 1;
        memset(risposta, 0, sizeof(risposta));

        do{
            //stampo risposta
            if(rxb_readline(&rxb, sd, risposta, &risposta_len) < 0){
                rxb_destroy(&rxb);
                close(sd);
                fprintf(stderr, "Connessione chiusa dal server\n");
                exit(EXIT_FAILURE);
            }
            puts(risposta);
        }while(strcmp(risposta, "fine\n") != 0);
        //debug
        char *debug = "Inserisci nuovo elemento\n";
        write(STDOUT_FILENO, debug, strlen(debug));

        //preparo per il prossimo input
        memset(input, 0, sizeof(input));
        if(fgets(input, sizeof(input), stdin) == NULL){
            perror("Errore fgets");
            close(sd);
            exit(EXIT_FAILURE);
        }
    }

    //invio fine a server
    write_all(sd, "\nfine\n", strlen("fine"));

    close(sd);
}
