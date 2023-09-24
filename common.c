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
    while (1){
        int BUFSIZE = sizeof(struct message);
        struct message message;
        memset(&message, 0, sizeof message);

        // waiting for client start the game
        char userCommand[11] = {};
        scanf("%s", userCommand);
        int start = !strcmp(userCommand, "start");
        int exit = !strcmp(userCommand, "exit");
        while (!start && !exit){
            scanf("%s", userCommand);
            start = !strcmp(userCommand, "start");
            exit = !strcmp(userCommand, "exit");
        }

        if (exit){
            message.type = EXIT_TYPE;
            ssize_t numBytesSent = send(serverSock, &message, sizeof(message), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");
            int errorShutdown = shutdown(serverSock, SHUT_RDWR);
            if (errorShutdown < 0)
                DieWithSystemMessage("shutdown() failed");
            return;
        }
        
        // init game
        message.type = START_TYPE;
        ssize_t numBytesSent = send(serverSock, &message, sizeof(message), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
        
        ssize_t numBytesRcvd = recv(serverSock, &message, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        printBoard(message);

        int isGameOver = 0;
        do{
            isGameOver = 0;
            char userCommand[12] = {};


            scanf("%s", userCommand);
            // commands list
            int start = !strcmp(userCommand, "start");
            int reveal = !strcmp(userCommand, "reveal");
            int flag = !strcmp(userCommand, "flag");
            int remove_flag = !strcmp(userCommand, "remove_flag");
            int reset = !strcmp(userCommand, "reset");
            int exit = !strcmp(userCommand, "exit");


            int x = -1;
            int y = -1;
            int hasError = 0;

            if (reveal){
                scanf("%d, %d", &x, &y );
                if (x > 3 || x < 0 || y > 3 || y < 0){
                    printf("error: invalid cell\n");
                    hasError = 1;
                }
                else if (message.board[x][y] >= 0){
                    printf("error: cell already revealed\n");
                    hasError = 1;
                }
                else {
                    // no errors
                    message.type = REVEAL_TYPE;
                    message.coordinates[0] = x;
                    message.coordinates[1] = y;
                }
                
            }
            else if (flag){
                scanf("%d, %d", &x, &y );
                if (x > 3 || x < 0 || y > 3 || y < 0){
                    printf("error: invalid cell\n");
                    hasError = 1;
                }
                else if (message.board[x][y] == FLAG_INT){
                    printf("error: cell already has a flag\n");
                    hasError = 1;
                }
                else if (message.board[x][y] >= -1){
                    printf("error: cannot insert flag in revealed cell\n");
                    hasError = 1;
                }
                else {
                    // no errors
                    message.type = FLAG_TYPE;
                    message.coordinates[0] = x;
                    message.coordinates[1] = y;
                }
            }
            else if(remove_flag){
                scanf("%d, %d", &x, &y );
                if (x > 3 || x < 0 || y > 3 || y < 0){
                    printf("error: invalid cell\n");
                    hasError = 1;
                }
                else if (message.board[x][y] != FLAG_INT){
                    printf("error: there is no flag in this cell\n");
                    hasError = 1;
                }
                else {
                    // no erros
                    message.type = REMOVE_FLAG_TYPE;
                    message.coordinates[0] = x;
                    message.coordinates[1] = y;
                }
            }
            else if(reset){
                message.type = RESET_TYPE;
            }
            else if(exit){
                message.type = EXIT_TYPE;
                ssize_t numBytesSent = send(serverSock, &message, sizeof(message), 0);
                if (numBytesSent < 0)
                    DieWithSystemMessage("send() failed");
                int errorShutdown = shutdown(serverSock, SHUT_RDWR);
                if (errorShutdown < 0)
                    DieWithSystemMessage("shutdown() failed");
                return;
            }
            else {
                printf("error: command not found\n");
                hasError = 1;
            }
            
            // handle erros on client side
            if (!hasError){
                ssize_t numBytesSent = send(serverSock, &message, sizeof(message), 0);
                if (numBytesSent < 0)
                    DieWithSystemMessage("send() failed");
                
                
                ssize_t numBytesRcvd = recv(serverSock, &message, BUFSIZE, 0);
                if (numBytesRcvd < 0)
                    DieWithSystemMessage("recv() failed");

                if(message.type == GAME_OVER_TYPE){
                    isGameOver = 1;
                    printf("GAME OVER!\n");
                }
                else if(message.type == WIN_TYPE){
                    isGameOver = 1;
                    printf("YOU WIN!\n");
                }
                printBoard(message);
            }
            
        }while(!isGameOver);   
    }
}

void printBoard(struct message message){
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

//------------------//
// SERVER FUNCTIONS //
//------------------//
void HandleTCPClient(int clntSock, const char* gamePath){

    int BUFSIZE = sizeof(struct message);
    struct message serverGameBoard;
    struct message userGameBoard;

    // loading board
    int fileResponse = initGameBoard(&serverGameBoard, gamePath);
    int numOfBombs = coutBombsOfBoard(serverGameBoard.board);
    if (fileResponse < 0)
        DieWithSystemMessage("open(<input_file>) failed\n");

    int clientCloseSocket = 0;
    do { 

        clientCloseSocket = 0;
        int numBytesRcvd = recv(clntSock, &userGameBoard, BUFSIZE, 0);
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

            userGameBoard.board[x][y] = serverGameBoard.board[x][y];


            if (serverGameBoard.board[x][y] == BOMB_INT){
                userGameBoard.type = GAME_OVER_TYPE;
                for (int i = 0; i < 4; i++){
                    userGameBoard.board[i][0] = serverGameBoard.board[i][0];
                    userGameBoard.board[i][1] = serverGameBoard.board[i][1];
                    userGameBoard.board[i][2] = serverGameBoard.board[i][2];
                    userGameBoard.board[i][3] = serverGameBoard.board[i][3];
                };
            }
            else {
                int numOfNotReveladeCells = 0;
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 4; j++)
                        if (userGameBoard.board[i][j] == OCULT_CELL_INT || userGameBoard.board[i][j] == FLAG_INT)
                            numOfNotReveladeCells++;
                
                int isWin = (numOfNotReveladeCells == numOfBombs);
                
                if (isWin){
                    userGameBoard.type = WIN_TYPE;
                    for (int i = 0; i < 4; i++){
                        userGameBoard.board[i][0] = serverGameBoard.board[i][0];
                        userGameBoard.board[i][1] = serverGameBoard.board[i][1];
                        userGameBoard.board[i][2] = serverGameBoard.board[i][2];
                        userGameBoard.board[i][3] = serverGameBoard.board[i][3];
                    };
                }
            }
        }
        else if (userGameBoard.type == FLAG_TYPE){
            int x = userGameBoard.coordinates[0];
            int y = userGameBoard.coordinates[1];
            userGameBoard.board[x][y] = -3;
        }
        else if (userGameBoard.type == REMOVE_FLAG_TYPE){
            int x = userGameBoard.coordinates[0];
            int y = userGameBoard.coordinates[1];
            userGameBoard.board[x][y] = -2;
        }
        else if (userGameBoard.type == RESET_TYPE){
            // preparing board to play
            printf("starting new game\n");  
            memset(&userGameBoard, 0, sizeof(userGameBoard));
            for (int i = 0; i < 4; i++){
                userGameBoard.board[i][0] = -2;
                userGameBoard.board[i][1] = -2;
                userGameBoard.board[i][2] = -2;
                userGameBoard.board[i][3] = -2;
            };
        }
        else if (userGameBoard.type == EXIT_TYPE){
            printf("client disconnected\n");
            clientCloseSocket = 1;
        }
        

        // send board to play
        ssize_t numBytesSent = send(clntSock, &userGameBoard, sizeof(userGameBoard), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
    }  while (!clientCloseSocket);

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

int coutBombsOfBoard(int board[4][4]) {
    int count = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            if (board[i][j] == -1)
                count++;
    return count;
}