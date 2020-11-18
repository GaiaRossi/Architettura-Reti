#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv){
    //controllo argomenti
    //./client hostname porta stringa1 stringa2
    if(argc != 5){
        fprintf(stderr, "Uso: ./client hostname porta stringa1 stringa2\n");
        exit(EXIT_FAILURE);
    }

    //creazione buffers
    char outBuff[2048], inBuff[2048];

    //creazione strutture per risoluzione nomi
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //risoluzione nomi
    int err;
    if((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0){
        fprintf(stderr, "Errore risoluzione nome\n");
        exit(EXIT_FAILURE);
    }

    //creazione socket
    int sd;
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    //connessione al server
    if(connect(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore nella connessione");
        close(sd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    //scrittura al server
    memset(&outBuff, 0, sizeof(outBuff));
    write(sd, outBuff, strlen(outBuff));

    //ricezione dal server
    memset(&inBuff, 0, sizeof(inBuff));
    read(sd, inBuff, sizeof(inBuff));
    write(STDOUT_FILENO, inBuff, strlen(inBuff));

    close(sd);
}