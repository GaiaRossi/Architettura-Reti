#define _POSIX_C_SOURCE	200809L
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
#include <fcntl.h>
#include <wait.h>

int status;

void child_handler(int signo){
    wait(&status);
}

int main(int argc, char** argv){
    //controllo argomenti
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
        exit(EXIT_FAILURE);
    }

    //gestione figli
    signal(SIGCHLD, child_handler);

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int err;
    if((err = getaddrinfo(NULL, argv[1], &hints, &res)) < 0){
        fprintf(stderr, "Errore getaddrinfo\n");
        exit(EXIT_FAILURE);
    }

    int sd;
    if((sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0){
        perror("Errore creazione socket");
        exit(EXIT_FAILURE);
    }

    if(bind(sd, res->ai_addr, res->ai_addrlen) < 0){
        perror("Errore bind");
        close(sd);
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(res);

    if(listen(sd, SOMAXCONN) < 0){
        perror("Errore listen");
        close(sd);
        exit(EXIT_FAILURE);
    }

    int nuovo_sd, pid;
    char yes[2] = "Y\0";
    char no[2] = "N\0";
    for(;;){
        nuovo_sd = accept(sd, NULL, NULL);
        pid = fork();
        if(pid < 0){
            perror("Errore fork");
            close(sd);
            close(nuovo_sd);
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            //codice figlio
            //attendo nome file
            char nomeFile[2048];
            memset(nomeFile, 0, sizeof(nomeFile));
            read(nuovo_sd, nomeFile, sizeof(nomeFile) - 1);

            //verifico esistenza file
            int fd;
            if((fd = open(nomeFile, O_RDONLY)) < 0){
                //file non esiste
                write(nuovo_sd, no, strlen(no));
                close(nuovo_sd);
                exit(EXIT_SUCCESS);
            }
            //il file esiste
            close(fd);
            write(nuovo_sd, yes, strlen(yes));

            //attendo stringa
            char stringa[2048];
            memset(stringa, 0, sizeof(stringa));
            read(nuovo_sd, stringa, sizeof(stringa) - 1);

            //redirezione output
            close(1);
            dup(nuovo_sd);
            close(nuovo_sd);

            execlp("grep", "grep", stringa, nomeFile, (char *)0);
            exit(EXIT_FAILURE);

        }
        //codice padre nel for
        close(nuovo_sd);
    }
}