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
    /* rgrep hostname porta stringa nomefile */
    /* controllo argomenti */
    if(argc != 5){
        fprintf(stderr, "Uso: ./rgrep hostname porta stringa nomefile\n");
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
    rxb_t rxb;
    char netIn[DIMENSIONE_BUFFER];
    char netOut[DIMENSIONE_BUFFER];
    memset(netIn, 0, sizeof(netIn));
    memset(netOut, 0, sizeof(netOut));

    rxb_init(&rxb, DIMENSIONE_BUFFER);

    /* invio nome file e stringa */
    snprintf(netOut, sizeof(netOut), "%s\n%s\n", argv[4], argv[3]);
    
    if(write_all(sd, netOut, strlen(netOut)) < 0){
        perror("write all");
        close(sd);
        exit(EXIT_FAILURE);
    }

    /* attendo risposta o eventuale
    *  chiusura della connessione 
    * perché non esiste il file 
    */

    size_t netIn_len;

    for(;;){
        netIn_len = sizeof(netIn) - 1;
        memset(netIn, 0, sizeof(netIn));
        if(rxb_readline(&rxb, sd, netIn, &netIn_len) < 0){
            break;
        }

        if(strcmp(netIn, "fine_richiesta") == 0){
            break;
        }

        puts(netIn);
    }
    rxb_destroy(&rxb);
    close(sd);
    exit(EXIT_SUCCESS);
}