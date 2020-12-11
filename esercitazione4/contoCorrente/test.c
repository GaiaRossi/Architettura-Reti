#include <stdlib.h>
#include <stdio.h>

int main(){
    char parola[100];

    fgets(parola, sizeof(parola), stdin);
    //printf("%s", parola);
    puts(parola);
}