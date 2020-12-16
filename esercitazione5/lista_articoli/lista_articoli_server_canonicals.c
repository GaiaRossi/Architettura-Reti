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
#include "rxb.h"

#define DIMENSIONE_BUFFER 4096

int status;

void child_handler(int signo){
    while(waitpid(-1, &status, WNOHANG) > 0);
}

int main(int argc, char** argv){
    /* controllo argomenti */
    if(argc != 2){
        fprintf(stderr, "Uso: ./server porta\n");
        exit(EXIT_FAILURE);
    }

    /* installo segnali */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_flags   = SA_RESTART;
    sa.sa_handler = child_handler;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
            perror("sigaction");
            exit(EXIT_FAILURE);
    }

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
    for(;;){
        if((nuovo_sd = accept(sd, NULL, NULL)) < 0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        if((pid = fork()) < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0){
            /* codice figlio */
            close(sd);

            /* disinstallo segnali */
            memset(&sa, 0, sizeof(sa));
            sigemptyset(&sa.sa_mask);
            sa.sa_handler = SIG_DFL;

            if (sigaction(SIGCHLD, &sa, NULL) == -1) {
                perror("sigaction");
                exit(EXIT_FAILURE);
            }

            /* preparazione buffers */
            char netIn[DIMENSIONE_BUFFER];
            char email[DIMENSIONE_BUFFER];
            char password[DIMENSIONE_BUFFER];
            char rivista[DIMENSIONE_BUFFER];

            memset(netIn, 0, sizeof(netIn));
            memset(email, 0, sizeof(email));
            memset(password, 0, sizeof(password));
            memset(rivista, 0, sizeof(rivista));

            /* leggo stringhe */
            read(nuovo_sd, netIn, sizeof(netIn) - 1);

            /* estrazione stringhe */
            /* controllo esistenza parentesi */
            char *parentesi = NULL;
            parentesi = memchr(netIn, '(', sizeof(netIn));
            if(parentesi == NULL){
                fprintf(stderr, "La stringa non rispetta il protocollo\n");
                close(sd);
                exit(EXIT_FAILURE);
            }

            parentesi = NULL;
            parentesi = memchr(netIn, ')', sizeof(netIn));

            if(parentesi == NULL){
                fprintf(stderr, "La stringa non rispetta il protocollo\n");
                close(sd);
                exit(EXIT_FAILURE);
            }

            int current_buffer = 0;
            char* buffers[3] = {email, password, rivista};
            int offset = 1;
            char *ptr;
            while(offset < parentesi - netIn){
                int lenght = strtol(&netIn[offset], &ptr, 10);
                int current = offset + 2;
                int i = 0;
                while(current <= offset + 1 + lenght){
                    buffers[current_buffer][i] = netIn[current];
                    i++;
                    current++;
                }
                current_buffer++;
                offset += 2 + lenght;
            }

            /* preparo pipe per nipoti */
            int pipefd[2];

            if(pipe(pipefd) < 0){
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            int pidn;
            if((pidn = fork()) < 0){
                perror("Nipote grep");
                exit(EXIT_FAILURE);
            }
            else if(pidn == 0){
                /* grep */
                close(pipefd[0]);
                close(nuovo_sd);

                close(1);
                dup(pipefd[1]);
                close(pipefd[1]);

                execlp("grep", "grep", email, "file_riviste.txt", (char *)0);
                exit(EXIT_FAILURE);
            }

            /* preparo pipe */
            int pipefd1[2];

            if(pipe(pipefd1) < 0){
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            if((pidn = fork()) < 0){
                perror("Nipote grep2");
                exit(EXIT_FAILURE);
            }
            else if(pidn == 0){
                /* grep */
                close(pipefd[1]);
                close(0);
                dup(pipefd[0]);
                close(pipefd[0]);

                close(pipefd1[0]);
                close(1);
                dup(pipefd1[1]);
                close(pipefd1[1]);

                execlp("grep", "grep", rivista, (char *)0);
                exit(EXIT_FAILURE);
            }
            
            close(pipefd[0]);
            close(pipefd[1]);

            int pipe_padre[2];
            if(pipe(pipe_padre) < 0){
                perror("pipe padre");
                exit(EXIT_FAILURE);
            }

            if((pidn = fork()) < 0){
                perror("Nipote sort");
                exit(EXIT_FAILURE);
            }
            else if(pidn == 0){
                /* sort */
                close(pipefd1[1]);
                close(pipe_padre[0]);

                close(0);
                dup(pipefd1[0]);
                close(pipefd1[0]);

                close(1);
                dup(pipe_padre[1]);
                close(pipe_padre[1]);

                execlp("sort", "sort", "-rn", (char *)0);
                exit(EXIT_FAILURE);
            }

            close(pipefd1[0]);
            close(pipefd1[1]);
            close(pipe_padre[1]);
            
            wait(&status);
            wait(&status);
     
            char buffer[DIMENSIONE_BUFFER];
            memset(buffer, 0, sizeof(buffer));
            read(pipe_padre[0], buffer, sizeof(buffer) - 1);

            int output_len = strlen(buffer);
            char netOut[2 * DIMENSIONE_BUFFER];
            memset(netOut, 0, sizeof(netOut));

            snprintf(netOut, sizeof(netOut) - 1, "(%d:%s)", output_len, buffer);

            write_all(nuovo_sd, netOut, strlen(netOut));
            close(nuovo_sd);
            exit(EXIT_SUCCESS);
        }
        close(nuovo_sd);
    }
}