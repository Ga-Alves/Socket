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

void HandleTCPServer(int clntSock){
    int BUFSIZE = sizeof(struct message);
    struct message message;
    memset(&message, 0, sizeof message);

    // init game
    message.type = 0;
    ssize_t numBytesSent = send(clntSock, &message, sizeof(message), 0);
    if (numBytesSent < 0)
        DieWithSystemMessage("send() failed");

    do{
        ssize_t numBytesRcvd = recv(clntSock, &message, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");
        
        printBoard(message);
    }while(0);
}

void printBoard(struct message message){
    for (int i = 0; i < 4; i++){
        printf("%d, %d, %d, %d\n", message.board[i][0], message.board[i][1], message.board[i][2], message.board[i][3] );
    };
}

void HandleTCPClient(int clntSock, const char* gamePath){

    int BUFSIZE = sizeof(struct message);
    struct message serverGameBoard;
    struct message userGameBoard;

    // loading board
    int fileResponse = initGameBoard(&serverGameBoard, gamePath);
    if (fileResponse < 0)
        DieWithSystemMessage("open(<input_file>) failed\n");

    // waiting for init game request
    int numBytesRcvd = recv(clntSock, &userGameBoard, BUFSIZE, 0);
    if (numBytesRcvd < 0)
        DieWithSystemMessage("recv() failed");
    if(userGameBoard.type != 0)
        DieWithSystemMessage("player should start game");
    else{
        // send board to play
        memset(&userGameBoard.board, -2, sizeof(userGameBoard.board));

        for (int i = 0; i < 4; i++){
            userGameBoard.board[i][0] = -2;
            userGameBoard.board[i][1] = -2;
            userGameBoard.board[i][2] = -2;
            userGameBoard.board[i][3] = -2;
        };

        ssize_t numBytesSent = send(clntSock, &userGameBoard, sizeof(userGameBoard), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
    }
    
   do { 
        // See if there is more data to receive
        int numBytesRcvd = recv(clntSock, &userGameBoard, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        printf("useAction: %d\n", userGameBoard.type);
        
        // switch (userGameBoard.type){
        //     case 0:
                
        //         break;
            
        //     default:
        //     break;
        // }
        
        // ssize_t numBytesSent = send(clntSock, &serverGameBoard, sizeof(serverGameBoard), 0);
        // if (numBytesSent < 0)
        //     DieWithSystemMessage("send() failed");

        
    }  while (0);
    

}

int initGameBoard(struct message *serverGameBoard, const char* gamePath){
    FILE *gameConfig = fopen(gamePath, "r");
    if (gameConfig == NULL){
        return -1;
    }
    int read[4][4] = {};
    for (int i = 0; i < 4; i++){
        fscanf(gameConfig, "%d, %d, %d, %d", &read[i][0], &read[i][1], &read[i][2], &read[i][3] );
    };
    fclose(gameConfig);
}