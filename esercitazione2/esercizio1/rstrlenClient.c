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
    //controllo argomenti
    //client hostname porta
    if(argc != 3){
        fprintf(stderr, "Uso: ./client hostname porta\n");
        exit(EXIT_FAILURE);
    }

    //creazione buffer invio
    char buffer[2048];

    //creazione variabili per risoluzione nomi
    struct addrinfo hints, *res;

    //pulizia hints
    memset(&hints, 0,sizeof(hints));

    //popolo hints
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //non necessito di ricercare una socket passiva
    
    //risoluzione nome
    int err;

    if((err = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0){
        fprintf(stderr, "Errore nella risoluzione del nome\n");
        exit(EXIT_FAILURE);
    }

    //creazione socket
    int sd;

    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore nella creazione della socket");
        exit(EXIT_FAILURE);
    }

    //non ho bisogno di una bind perché non sono
    //io client a dovermi agganciare alla socket
    //voglio solo la risoluzione del nome in res

    freeaddrinfo(res);

    //connessione al server
    if(connect(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore nella connessione con il server");
        exit(EXIT_FAILURE);
    }

    //creazione buffer risposta
    char risposta[2048];
    
    //ciclo per lettura stringhe
    memset(&buffer, 0, sizeof(buffer));
    scanf("%s", buffer);

    while(strcmp(buffer, "fine") != 0){
        //debug
        printf("Invio: %s\n", buffer);

        //invio al server
        write(sd, buffer, sizeof(buffer));

        //leggo dal server e mostro a video
        memset(&risposta, 0, sizeof(risposta));
        int nread;
        nread = read(sd, risposta, sizeof(risposta));
        printf("Letti %d caratteri\n", nread);
        write(STDOUT_FILENO, risposta, sizeof(risposta));
        printf("\n");

        //nuovo input
        memset(&buffer, 0, sizeof(buffer));
        scanf("%s", buffer);
    }

    //chiusura socket
    if(shutdown(sd, SHUT_WR) < 0){
        perror("Errore chiusura connessione");
        close(sd);
        exit(EXIT_FAILURE);
    }
}