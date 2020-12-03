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
    //./client localhost porta opzioni
    //controllo argomenti
    if(argc <= 2){
        fprintf(stderr, "Uso: ./client nomeHost porta <opzioni>\n");
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

    int sd;
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    if(connect(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore connessione");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    //invio il numero di opzioni inserite
    int n_opzioni = argc - 3;
    char buffer[2048];
    snprintf(buffer, sizeof(buffer), "%d", n_opzioni);
    write(sd, buffer, strlen(buffer));
    //lettura ack
    read(sd, buffer, sizeof(buffer));

    if(n_opzioni > 0){
        char opzioni[2048];
        snprintf(opzioni, sizeof(opzioni), "%s", argv[3]);
        int i;
        for(i = 4; i < argc; i++){
            strcat(opzioni, argv[i]);
            strcat(opzioni, " ");
        }
        write(sd, opzioni, strlen(opzioni));
    }

    //lettura risposta
    char risposta[2048];
    memset(&risposta, 0, sizeof(risposta));
    read(sd, risposta, sizeof(risposta) - 1);
    printf("%s\n", risposta);

    close(sd);
}