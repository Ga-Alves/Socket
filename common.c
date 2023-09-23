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

//------------------//
// CLIENT FUNCTIONS //
//------------------//
void HandleTCPServer(int serverSock){
    int BUFSIZE = sizeof(struct message);
    struct message message;
    memset(&message, 0, sizeof message);

    char userCommand[11] = {};

    scanf("%s", userCommand);
    int start = !strcmp(userCommand, "start");
    if (start){
        // init game
        message.type = START_TYPE;
        ssize_t numBytesSent = send(serverSock, &message, sizeof(message), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
        
        ssize_t numBytesRcvd = recv(serverSock, &message, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        printBoard(message);
    }
    else {
        DieWithSystemMessage("you should start the game first!");
    }


    do{
        char userCommand[11] = {};
        int x = -1;
        int y = -1;

        scanf("%s %d, %d", userCommand, &x, &y );

        int start = !strcmp(userCommand, "start");
        int reveal = !strcmp(userCommand, "reveal");
        // int flag = !strcmp(userCommand, "flag");
        // int remove_flag = !strcmp(userCommand, "remove_flag");
        // int reset = !strcmp(userCommand, "reset");
        int exit = !strcmp(userCommand, "exit");

        if (reveal){
            message.type = REVEAL_TYPE;
            message.coordinates[0] = x;
            message.coordinates[1] = y;
        }

        ssize_t numBytesSent = send(serverSock, &message, sizeof(message), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
        
        ssize_t numBytesRcvd = recv(serverSock, &message, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        printBoard(message);
    }while(1);
}

void printBoard(struct message message){
    for (int i = 0; i < 4; i++){
        for (int j = 0; j < 4; j++){
            switch (message.board[i][j])
            {
            case -1:
                printf("* ");
                break;
            case -2:
                printf("- ");
                break;
            case -3:
                printf("> ");
                break;
            default:
                printf("%d ", message.board[i][j]);
                break;
            }
        }
        printf("\n");
    };
}

//------------------//
// SERVER FUNCTIONS //
//------------------//
void HandleTCPClient(int clntSock, const char* gamePath){

    int BUFSIZE = sizeof(struct message);
    struct message serverGameBoard;
    struct message userGameBoard;

    // loading board
    int fileResponse = initGameBoard(&serverGameBoard, gamePath);
    if (fileResponse < 0)
        DieWithSystemMessage("open(<input_file>) failed\n");

    
    
   do { 
        // See if there is more data to receive
        printf("-------h-----------\n");
      /**/  int numBytesRcvd = recv(clntSock, &userGameBoard, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        if (userGameBoard.type == START_TYPE){
            // preparing board to play
            memset(&userGameBoard, 0, sizeof(userGameBoard));
            for (int i = 0; i < 4; i++){
                userGameBoard.board[i][0] = -2;
                userGameBoard.board[i][1] = -2;
                userGameBoard.board[i][2] = -2;
                userGameBoard.board[i][3] = -2;
            };

        }
        else if (userGameBoard.type == REVEAL_TYPE){
            int x = userGameBoard.coordinates[0];
            int y = userGameBoard.coordinates[1];
            printf("cooordenaadas: %d %d\n", x, y);
            userGameBoard.board[x][y] = serverGameBoard.board[x][y];
        }

        // send board to play
        ssize_t numBytesSent = send(clntSock, &userGameBoard, sizeof(userGameBoard), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
    }  while (1);
    

}

int initGameBoard(struct message *serverGameBoard, const char* gamePath){
    FILE *gameConfig = fopen(gamePath, "r");
    if (gameConfig == NULL){
        return -1;
    }

    for (int i = 0; i < 4; i++){
        fscanf(gameConfig, "%d, %d, %d, %d", &serverGameBoard->board[i][0], &serverGameBoard->board[i][1], &serverGameBoard->board[i][2], &serverGameBoard->board[i][3] );
    };
    fclose(gameConfig);
}