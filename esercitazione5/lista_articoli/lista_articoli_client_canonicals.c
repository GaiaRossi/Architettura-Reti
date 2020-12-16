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

//lista_articoli   server   porta
#define DIMENSIONE_BUFFER 4096

void removeNewLine(char* src, char* dest){
        /* rimozione a capo */
        char *stringa_restante = NULL;
        stringa_restante = memchr(src, '\n', strlen(src));

        size_t corretti = stringa_restante - src;
        memcpy(dest, src, corretti);
}

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

    char buffer[DIMENSIONE_BUFFER];
    memset(buffer, 0, sizeof(buffer));

    char input[DIMENSIONE_BUFFER];
    memset(input, 0, sizeof(input));
    char email[DIMENSIONE_BUFFER];
    char password[DIMENSIONE_BUFFER];
    char rivista[DIMENSIONE_BUFFER];

    memset(email, 0, sizeof(email));
    memset(password, 0, sizeof(password));
    memset(rivista, 0, sizeof(rivista));

    char netOut[4 * DIMENSIONE_BUFFER];
    memset(netOut, 0, sizeof(netOut));

    for(;;){
        printf("Inserisci la mail:\n");
        fgets(input, sizeof(input) - 1, stdin);
        removeNewLine(input, email);
        if(strcmp(input, "fine")){
            break;
        }
        
        memset(input, 0, sizeof(input));

        printf("Inserisci la password:\n");
        read(STDIN_FILENO, input, sizeof(input) - 1);
        if(strcmp(input, "fine")){
            break;
        }
        removeNewLine(input, password);
        
        memset(input, 0, sizeof(input));

        printf("Inserisci la rivista:\n");
        read(STDIN_FILENO, input, sizeof(input) - 1);
        if(strcmp(input, "fine")){
            break;
        }
        removeNewLine(input, rivista);

        int email_len = strlen(email);
        int password_len = strlen(password);
        int rivista_len = strlen(rivista);

        /* creazione stringa da mandare a server */
        snprintf(netOut, sizeof(netOut), "(%d:%s%d:%s%d:%s)", email_len, email, password_len, password, rivista_len, rivista);
        write_all(sd, netOut, strlen(netOut));
          
        /* leggo la risposta */
        memset(buffer, 0, sizeof(buffer));
        read(sd, buffer, sizeof(buffer));
        /* controllo esistenza parentesi */
        char *parentesi = NULL;
        parentesi = memchr(buffer, '(', sizeof(buffer));
        if(parentesi == NULL){
            fprintf(stderr, "La stringa non rispetta il protocollo\n");
            close(sd);
            exit(EXIT_FAILURE);
        }

        parentesi = NULL;
        parentesi = memchr(buffer, ')', sizeof(buffer));

        if(parentesi == NULL){
            fprintf(stderr, "La stringa non rispetta il protocollo\n");
            close(sd);
            exit(EXIT_FAILURE);
        }

        /* leggo stringhe */
        int offset = 1;
        char *ptr;
        char tmp[DIMENSIONE_BUFFER];
        memset(tmp, 0, sizeof(tmp));
        while(offset < parentesi - buffer){
            int lenght = strtol(&buffer[offset], &ptr, 10);
            int current = offset + 2;
            int i = 0;
            memset(tmp, 0, sizeof(tmp));
            while(current <= offset + 1 + lenght){
                tmp[i] = buffer[current];
                i++;
                current++;
            }
            printf("%s\n", tmp);
            offset += 2 + lenght;
        } 
    }
    close(sd);
}