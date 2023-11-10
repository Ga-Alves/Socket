#include "common.h"

void DieWithUserMessage(const char *msg, const char *detail) {
fputs(msg, stderr);
fputs(": ", stderr);
fputs(detail, stderr);
fputc('\n', stderr);
exit(1);
}

void DieWithSystemMessage(const char *msg) {
perror(msg);
exit(1);
}


char* substring(int init, int end, char string[1000]){
    char *sub = malloc(sizeof(char[1000]));

    for (int i = 0; i < end - init; i++){
        sub[i] = string[init + i];
    }
    sub[end-init] = '\0';
    
    return sub;
}