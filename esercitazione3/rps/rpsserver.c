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
#include <wait.h>

int status;

void child_handler(int signo){
    wait(&status);
}

int main(int argc, char** argv){
    //./server porta
    //controllo argomenti
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
        exit(EXIT_FAILURE);
    }

    //da cambiare con sigaction
    signal(SIGCHLD, child_handler);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if((err = getaddrinfo(NULL, argv[1], &hints, &res)) != 0){
        fprintf(stderr, "Errore getaddrinfo\n");
        exit(EXIT_FAILURE);
    }

    int sd;
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    if(bind(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore nel bind");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if(listen(sd, SOMAXCONN) < 0){
        perror("Errore listen");
        exit(EXIT_FAILURE);
    }

    while (true){
        struct sockaddr_storage client;
        socklen_t client_len = sizeof(client);
        int nuovo_sd;
        if((nuovo_sd = accept(sd, (struct sockaddr *)&client, &client_len)) < 0){
            perror("Errore accept");
            exit(EXIT_FAILURE);
        }
        //creazione figlio per gestire connessione
        int pid;
        if((pid = fork()) < 0){
            perror("Errore creazione figlio");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            //codice figlio
            close(sd);

            //attendo il numero delle opzioni
            char buffIn[2048];
            read(nuovo_sd, buffIn, sizeof(buffIn) - 1);
            //invio ack
            write(nuovo_sd, buffIn, strlen(buffIn));

            int n_opzioni = atoi(buffIn);

            //redirezione output per ps
            close(1);
            dup(nuovo_sd);
            close(2);
            dup(nuovo_sd);
            close(nuovo_sd);
          
            if(n_opzioni > 0){
                char opzioni[2048];
                memset(&opzioni, 0, sizeof(opzioni));
                read(nuovo_sd, opzioni, sizeof(opzioni) - 1);
                execlp("ps", "ps", opzioni, (char *)0);
            }
            else{
                execlp("ps", "ps", (char *)0);
            }

            exit(EXIT_FAILURE);
        }
        //codice padre nel while
        close(nuovo_sd);
    }

}