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
    /* coffee_machines server porta */
    /* controllo argomenti */
    if(argc != 3){
        fprintf(stderr, "Uso: ./coffee_machines server porta\n");
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
    char username[DIMENSIONE_BUFFER];
    char password[DIMENSIONE_BUFFER];
    char categoria[DIMENSIONE_BUFFER];
    char netOut[DIMENSIONE_BUFFER];
    char risposta[DIMENSIONE_BUFFER];

    rxb_t rxb;
    rxb_init(&rxb, DIMENSIONE_BUFFER);

    /* leggo input, preparo stringa, invio stringa, attendo risposta */
    /* mi autentico una volta soltanto */
    memset(username, 0, sizeof(username));
    printf("Inserisci lo username:\n");
    if(fgets(username, sizeof(username), stdin) == NULL){
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    if(strcmp(username, "fine\n") == 0){
        close(sd);
        exit(EXIT_SUCCESS);
    }

    memset(password, 0, sizeof(password));
    printf("Inserisci la password:\n");
    if(fgets(password, sizeof(password), stdin) == NULL){
        perror("fgets");
        exit(EXIT_FAILURE);
    }

    if(strcmp(password, "fine\n") == 0){
        close(sd);
        exit(EXIT_SUCCESS);
    }

    /* preparo stringa per inviare credenziali */
    memset(netOut, 0, sizeof(netOut));
    strcpy(netOut, username);
    strcat(netOut, password);

    /* invio credenziali */
    if(write_all(sd, netOut, strlen(netOut)) < 0){
        perror("write all");
        exit(EXIT_FAILURE);
    }

    /* controllo se autenticato o meno */
    size_t risposta_len;

    risposta_len = sizeof(risposta) - 1;
    memset(risposta, 0, sizeof(risposta));

    if(rxb_readline(&rxb, sd, risposta, &risposta_len) < 0){
        /* server che ha chiuso la connessione */
        rxb_destroy(&rxb);
        exit(EXIT_FAILURE);
    }

    /* analizzo ack */
    if(strcmp(risposta, "OK") == 0){
        printf("Autenticato\n");
    }
    else if(strcmp(risposta, "NO") == 0){
        printf("Non autenticato\n");
        exit(EXIT_FAILURE);
    }
    else{
        fprintf(stderr, "Valore di ack sbagliato!\n");
        exit(EXIT_FAILURE);
    }

    /* leggo categoria */
    for(;;){
        memset(categoria, 0, sizeof(categoria));
        printf("Inserisci la categoria delle macchine da cercare:\n");
        if(fgets(categoria, sizeof(categoria), stdin) == NULL){
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        /* controllo se l'utente ha inserito fine */
        if(strcmp(categoria, "fine\n") == 0){
            close(sd);
            exit(EXIT_SUCCESS);
        }

        /* invio a server */
        if(write_all(sd, categoria, strlen(categoria)) < 0){
            perror("write all");
            exit(EXIT_FAILURE);
        }

        /* attendo risposta */
        for(;;){
            risposta_len = sizeof(risposta) - 1;
            memset(risposta, 0, sizeof(risposta));

            if(rxb_readline(&rxb, sd, risposta, &risposta_len) < 0){
                rxb_destroy(&rxb);
                exit(EXIT_FAILURE);
            }

            /* controllo se il server ha finito di
            * inviare dati */
            if(strcmp(risposta, "fine_richiesta") == 0){
                break;
            }

            /* stampo risposta se dati importanti */
            puts(risposta);
        }
    }

    rxb_destroy(&rxb);
    close(sd);
    exit(EXIT_SUCCESS);
}