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

void printBoard(struct action message){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            switch (message.board[i][j])
            {
            case -1:
                printf("*\t\t");
                break;
            case -2:
                printf("-\t\t");
                break;
            case -3:
                printf(">\t\t");
                break;
            default:
                printf("%d\t\t", message.board[i][j]);
                break;
            }
        }
        printf("\n");
    };
}