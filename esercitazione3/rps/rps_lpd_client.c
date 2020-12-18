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

#define DIMENSIONE_BUFFER 2048

int main(int argc, char **argv){
    /* rps server porta opzioni*/
    /* controllo argomenti */
    if(argc < 3){
        fprintf(stderr, "Uso: ./client_lpd server porta [opzioni]\n");
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

    /* preparo buffers */
    char netOut[DIMENSIONE_BUFFER];
    char netIn[DIMENSIONE_BUFFER];

    uint8_t netOut_len[2];
    uint8_t netIn_len[2];
    
    memset(netOut, 0, sizeof(netOut));
    memset(netIn, 0, sizeof(netIn));
    
    /* questo servizio non richiede il riuso della socket */
    int len;

    int i;
    for(i = 3; i < argc; i++){
        strcat(netOut, argv[i]);
    }

    len = strlen(netOut);

    /* controllo se opzioni troppo lunghe */
    if(len > UINT16_MAX){
        fprintf(stderr, "Opzioni troppi grandi\n");
        close(sd);
        exit(EXIT_FAILURE);
    }

    netOut_len[0] = (len & 0xFF00) >> 8;
    netOut_len[1] = (len & 0xFF);

    /* invio lunghezza opzioni */
    if(write_all(sd, netOut_len, 2) < 0){
        perror("write all");
        close(sd);
        exit(EXIT_FAILURE);
    }

    /* invio opzioni */
    if(write_all(sd, netOut, len) < 0){
        perror("write all");
        close(sd);
        exit(EXIT_FAILURE);
    }

    /* lettura lunghezza risposta */
    if(read_all(sd, netIn_len, 2) < 0){
        perror("read all");
        close(sd);
        exit(EXIT_FAILURE);
    }

    len = netIn_len[0] << 8 | netIn_len[1];
    /* lettura risposta */
    int to_read = len;
    int nread;
    while(to_read > 0){
        size_t buf_size = sizeof(netIn);
        size_t sz = (to_read < buf_size) ? to_read : buf_size;

        nread = read(sd, netIn, sz);
        if(nread < 0){
            perror("read");
            close(sd);
            exit(EXIT_FAILURE);
        }

        if(write_all(1, netIn, nread) < 0){
            perror("write_all");
            close(sd);
            exit(EXIT_FAILURE);
        }

        to_read -= nread;
    }

    if(write_all(1, "fin\n", strlen("fin\n")) < 0){
            perror("write_all");
            close(sd);
            exit(EXIT_FAILURE);
        }

    close(sd);
}