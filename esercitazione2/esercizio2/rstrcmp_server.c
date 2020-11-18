#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv){
    //controllo argomenti
    //./Server porta
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
        exit(EXIT_FAILURE);
    }

    //creazione buffer richiesta e risposta
    char reqBuff[2048], resBuff[2048];

    //creazione strutture per risoluzione nome
    struct addrinfo hints, *res;

    //pulizia di hints e inserimento criteri
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //risoluzione nomi
    int err;
    if((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0){
        fprintf(stderr, "Errore nella risoluzione dei nomi");
        exit(EXIT_FAILURE);
    }

    //creazione socket
    int sd;
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione della socket");
        exit(EXIT_FAILURE);
    }

    //bind
    if(bind(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore binding");
        close(sd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    //trasformazione a socket passiva
    if(listen(sd, SOMAXCONN) < 0){
        perror("Errore ascolto");
        close(sd);
        exit(EXIT_FAILURE);
    }

    //accetto connessioni
    int nuovo_sd;
    struct sockaddr_storage client;
    socklen_t client_len = sizeof(client);
    if((nuovo_sd = accept(sd, (struct sockaddr *)&client, &client_len)) < 0){
        perror("Errore accept");
        close(sd);
        exit(EXIT_FAILURE);
    }

    //leggo dal client
    read(nuovo_sd, reqBuff, sizeof(reqBuff));

    write(STDOUT_FILENO, reqBuff, strlen(reqBuff));
    printf("\n");

    sprintf(resBuff, "connesso");

    //scrittura al client
    write(nuovo_sd, resBuff, strlen(resBuff));

    close(nuovo_sd);
    close(sd);
}