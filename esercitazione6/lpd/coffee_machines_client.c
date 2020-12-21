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
    char risposta[DIMENSIONE_BUFFER];
    uint8_t ack[4];

    int username_len, password_len, categoria_len, risposta_len;

    uint8_t len[2];

    memset(username, 0, sizeof(username));
    printf("Inserisci lo username:\n");
    scanf("%s", username);

    if(strcmp(username, "fine") == 0){
        len[0] = 0;
        len[1] = 0;
        
        if(write_all(sd, len, 2) < 0){
            perror("write all");
            exit(EXIT_FAILURE);
        }

        close(sd);
        exit(EXIT_SUCCESS);
    }

    /* calcolo lunghezza username */
    username_len = strlen(username);
    len[0] = (username_len & 0xFF00) >> 8;
    len[1] = username_len & 0xFF;

    /* invio lunghezza username */
    if(write_all(sd, len, 2) < 0){
        perror("write all");
        exit(EXIT_FAILURE);
    }

    /* invio username */
    if(write_all(sd, username, username_len) < 0){
        perror("write all");
        exit(EXIT_FAILURE);
    }

    /* leggo password */
    memset(password, 0, sizeof(password));
    printf("Inserisci la password:\n");
    scanf("%s", password);

    if(strcmp(password, "fine") == 0){
        len[0] = 0;
        len[1] = 0;
        
        if(write_all(sd, len, 2) < 0){
            perror("write all");
            exit(EXIT_FAILURE);
        }

        close(sd);
        exit(EXIT_SUCCESS);
    }

    /* calcolo lunghezza password */
    password_len = strlen(password);
    len[0] = (password_len & 0xFF00) >> 8;
    len[1] = (password_len & 0xFF);

    /* invio lunghezza password */
    if(write_all(sd, len, 2) < 0){
        perror("write all");
        exit(EXIT_FAILURE);
    }

    /* invio password */
    if(write_all(sd, password, password_len) < 0){
        perror("write all");
        exit(EXIT_FAILURE);
    }

    /* leggo ack per autenticazione */
    if(read_all(sd, ack, 4) < 0){
        perror("read all");
        exit(EXIT_FAILURE);
    }

    /* controllo ack */
    if((ack[0] != '0') | (ack[1] != '2') | (ack[2] != 'O') | (ack[3] != 'K')){
        fprintf(stderr, "Non autenticato\n");
        exit(EXIT_FAILURE);
    }
    else{
        printf("Autenticato\n");
    }

    /* richiedo categoria */
    for(;;){
        memset(categoria, 0, sizeof(categoria));

        printf("Inserisci la categoria:\n");
        scanf("%s", categoria);

        if(strcmp(categoria, "fine") == 0){
            len[0] = 0;
            len[1] = 0;
            
            if(write_all(sd, len, 2) < 0){
                perror("write all");
                exit(EXIT_FAILURE);
            }

            break;
        }

        categoria_len = strlen(categoria);
        len[0] = (categoria_len & 0xFF00) >> 8;
        len[1] = (categoria_len & 0xFF);

        /* invio lunghezza categoria */
        if(write_all(sd, len, 2) < 0){
            perror("write all");
            exit(EXIT_FAILURE);
        }

        /* invio categoria */
        if(write_all(sd, categoria, categoria_len) < 0){
            perror("write all");
            exit(EXIT_FAILURE);
        }

        /* leggo lunghezza risposta */
        if(read_all(sd, len, 2) < 0){
            perror("read all");
            exit(EXIT_FAILURE);
        }

        risposta_len = (len[0] << 8) | len[1];
        
        /* leggo risposta */
        int to_read = risposta_len;
        int nread;
        while(to_read > 0){
            memset(risposta, 0, sizeof(risposta));
            size_t buf_size = sizeof(risposta);
            size_t sz = (to_read < buf_size) ? to_read : buf_size;

            nread = read(sd, risposta, sz);
            if(nread < 0){
                perror("read");
                close(sd);
                exit(EXIT_FAILURE);
            }

            if(write_all(1, risposta, nread) < 0){
                perror("write_all");
                close(sd);
                exit(EXIT_FAILURE);
            }

            to_read -= nread;
        }
    }
    close(sd);
    exit(EXIT_SUCCESS);
}