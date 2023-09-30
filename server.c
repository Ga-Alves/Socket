#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define MAXPENDING 1

//server functions
void HandleTCPClient(int sock, const char* gamePath);
int initGameBoard(struct action *serverGameBoard, const char* gamePath);
int coutBombsOfBoard(int board[4][4]);


// ./server v4 51511 -i input/jogo.txt
int main(int argc, char const *argv[])
{

    if (argc < 5){
        printf("Quantidade de parâmetros errada!\n");
        return -1;
    }

    in_port_t servPort = atoi(argv[2]); // First arg: local port
    
    
    //---------------//
    // Caso for IPv4 //
    //---------------//
    if (!strcmp(argv[1], "v4")){
        // Create socket for incoming connections
        int servSock; // Socket descriptor for server
        if ((servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithSystemMessage("socket() failed");

        // Construct local address structure
        struct sockaddr_in servAddr; // Local address
        memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
        servAddr.sin_family = AF_INET; // IPv4 address family
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Any incoming interface
        servAddr.sin_port = htons(servPort); // Local port

        inet_pton(AF_INET, "192.168.0.182" , &(servAddr.sin_addr));

        // Bind to the local address
        if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
            DieWithSystemMessage("bind() failed");

        // linten channel
        if (listen(servSock, MAXPENDING) < 0)
            DieWithSystemMessage("listen() failed");

        while (1){
            struct sockaddr_in clntAddr; // Client address
            // Set length of client address structure (in-out parameter)
            socklen_t clntAddrLen = sizeof(clntAddr);
            // Wait for a client to connect
            int sock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (sock < 0)
                DieWithSystemMessage("accept() failed");
            
            printf("client connected\n");

            HandleTCPClient(sock, argv[4]);
        }

    }
    //---------------//
    // Caso for IPv6 //
    //---------------//
    else if(!strcmp(argv[1], "v6")){
        int servSock; // Socket descriptor for server
        if ((servSock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) < 0)
            DieWithSystemMessage("socket() failed");

        // Construct local address structure
        struct sockaddr_in6 servAddr; // Local address
        memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
        servAddr.sin6_family = AF_INET6; // IPv4 address family
        servAddr.sin6_addr = in6addr_any; // Any incoming interface
        servAddr.sin6_port = htons(servPort); // Local port

        // Bind to the local address
        if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
            DieWithSystemMessage("bind() failed");

        // linten channel
        if (listen(servSock, MAXPENDING) < 0)
            DieWithSystemMessage("listen() failed");

        while (1){
            struct sockaddr_in6 clntAddr; // Client address
            // Set length of client address structure (in-out parameter)
            socklen_t clntAddrLen = sizeof(clntAddr);
            // Wait for a client to connect
            int sock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (sock < 0)
                DieWithSystemMessage("accept() failed");
            
            printf("client connected\n");

            HandleTCPClient(sock, argv[4]);
        }

    }
        
    return 0;
}



//------------------//
// SERVER FUNCTIONS //
//------------------//
void HandleTCPClient(int sock, const char* gamePath){

    int BUFSIZE = sizeof(struct action);
    struct action serverGameBoard;
    struct action userGameBoard;

    // loading board
    int fileResponse = initGameBoard(&serverGameBoard, gamePath);
    int numOfBombs = coutBombsOfBoard(serverGameBoard.board);
    if (fileResponse < 0)
        DieWithSystemMessage("open(<input_file>) failed\n");

    int clientCloseSocket = 0;
    do { 

        clientCloseSocket = 0;
        int numBytesRcvd = recv(sock, &userGameBoard, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        
        int x = userGameBoard.coordinates[0];
        int y = userGameBoard.coordinates[1];

        //----------------//
        // Server Actions //
        //----------------//
        if (userGameBoard.type == START_TYPE){
            // preparing board to play
            memset(&userGameBoard, 0, sizeof(userGameBoard));
            userGameBoard.type = STATE_TYPE;
            for (int i = 0; i < 4; i++){
                userGameBoard.board[i][0] = -2;
                userGameBoard.board[i][1] = -2;
                userGameBoard.board[i][2] = -2;
                userGameBoard.board[i][3] = -2;
            };

        }
        else if (userGameBoard.type == REVEAL_TYPE){
            // mostra celula para jogador
            userGameBoard.type = STATE_TYPE;
            userGameBoard.board[x][y] = serverGameBoard.board[x][y];

            // Verifica Game Over
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
                // Verifica se Venceu com a atual jogada
                int numOfReveladeCells = 0;
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 4; j++)
                        if (userGameBoard.board[i][j] >= 0 )
                            numOfReveladeCells++;
                
                int isWin = ((16 - numOfReveladeCells) == numOfBombs);
                
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
            userGameBoard.type = STATE_TYPE;
            userGameBoard.board[x][y] = -3;
        }
        else if (userGameBoard.type == REMOVE_FLAG_TYPE){
            userGameBoard.type = STATE_TYPE;
            userGameBoard.board[x][y] = -2;
        }
        else if (userGameBoard.type == RESET_TYPE){
            // preparing board to play
            printf("starting new game\n");  
            memset(&userGameBoard, 0, sizeof(userGameBoard));
            userGameBoard.type = RESET_TYPE;
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
        ssize_t numBytesSent = send(sock, &userGameBoard, sizeof(userGameBoard), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
    }  while (!clientCloseSocket);

}

int initGameBoard(struct action *serverGameBoard, const char* gamePath){
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