#include <string.h>
#include <stdio.h>

int main(int argc, char **argv){
    char buffer[100] = "test\ntest\n";
    char *buffer2 = "ok\n";
    strcat(buffer, buffer2);
    printf("%s", buffer);
}