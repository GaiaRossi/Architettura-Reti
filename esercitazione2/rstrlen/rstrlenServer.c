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
    //rstrlen
    if(argc != 1){
        fprintf(stderr, "Uso: ./server\n");
        exit(EXIT_FAILURE);
    }
    //creazione buffer ricezione
    char buffer[2048];

    //creazione struct che conterrà i criteri
    //di scelta delle struct tornare da getaddrinfo
    struct addrinfo hints, *res;

    //prima di usare hints però bisogna pulirla
    memset(&hints, 0, sizeof(hints));

    //bisogna poi settare i criteri della socket che vogliamo
    //da getaddrinfo
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    //da capire
    hints.ai_flags = AI_PASSIVE;

    //variabile che contiene il valore di ritorno
    //delle funzioni per controllare il corretto funzionamento
    int err;

    //getaddrinfo ritorna un insieme di socket che hanno
    //il nome richiesto, il servizio richiesto e rispettano
    //i criteri specificati in hints
    //mettono l'inizio della lista in res
    if((err = getaddrinfo(NULL, "50001", &hints, &res)) != 0){
        fprintf(stderr, "Errore in getaddrinfo\n");
        exit(EXIT_FAILURE);
    }

    //variabile che conterrà il socket descriptor
    int sd;

    //creazione della socket
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione socket: ");
        exit(EXIT_FAILURE);
    }

    //bind della socket
    if(bind(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore nel binding: ");
        exit(EXIT_FAILURE);
    }

    //da capire
    freeaddrinfo(res);

    //trasformo la socket del server in una socket passiva
    if(listen(sd, SOMAXCONN) < 0){
        perror("Errore trasformazione socket passiva: ");
        exit(EXIT_FAILURE);
    }

    //ascolto infinito
    for(;;){
        //accettazione delle connessioni
        //creazione delle strutture necessarie per accept
        int nuovo_sd;
        //mette le informazioni del client connesso
        struct sockaddr_storage client_address;
        socklen_t client_address_len = sizeof(client_address);

        //accetto la connessione
        //sto controllando il valore di nuovo_sd
        if((nuovo_sd = accept(sd, (struct sockaddr *)&client_address, &client_address_len)) < 0){		
            perror("Errore nella connessione client server: ");
            exit(EXIT_FAILURE);
        }

        //client connesso
        //pulizia del buffer
        memset(buffer, 0, sizeof(buffer));
        //leggo finché c'è qualcosa
        while(read(nuovo_sd, buffer, sizeof(buffer)) > 0){
        /*
        //lettura stringa
        if(read(nuovo_sd, buffer, sizeof(buffer)) < 0){
            perror("Errore in lettura");
            close(nuovo_sd);
            exit(EXIT_FAILURE);
        }
        else{*/
            printf("Il client ha inviato: %s\n", buffer);
            char risposta[2048];
            memset(&risposta, 0, sizeof(risposta));
            sprintf(risposta, "%lu", strlen(buffer));
            printf("La stringa e' di lunghezza %s\n", risposta);
            //printf("La stringa e' di lunghezza %d\n", u8_mblen(buffer, sizeof(buffer)));
        //}

        //pulizia buffer dopo lettura
        memset(buffer, 0, sizeof(buffer));

        //invio della risposta
        write(nuovo_sd, risposta, strlen(risposta));

        }
        //chiusura connessione se
        //client chiude
        printf("Chiusa una connessione\n");
        close(nuovo_sd);
    }

}