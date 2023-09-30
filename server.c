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
void HandleTCPClient(int sock, struct action *serverGameboard);
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

        // inet_pton(AF_INET, "your_public_IP" , &(servAddr.sin_addr));

        // Bind to the local address
        if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
            DieWithSystemMessage("bind() failed");

        // linten channel
        if (listen(servSock, MAXPENDING) < 0)
            DieWithSystemMessage("listen() failed");


        // Criando o tabuleiro do servidor
        struct action serverGameboard;
        int fileResponse = initGameBoard(&serverGameboard, argv[4]);
        if (fileResponse < 0)
        DieWithSystemMessage("open(<input_file>) failed\n");
        printBoard(serverGameboard);

        while (1){
            struct sockaddr_in clntAddr; // Client address
            // Set length of client address structure (in-out parameter)
            socklen_t clntAddrLen = sizeof(clntAddr);
            // Wait for a client to connect
            int sock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (sock < 0)
                DieWithSystemMessage("accept() failed");
            
            printf("client connected\n");

            HandleTCPClient(sock, &serverGameboard);
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

        // Criando o tabuleiro do servidor
        struct action serverGameboard;
        int fileResponse = initGameBoard(&serverGameboard, argv[4]);
        if (fileResponse < 0)
        DieWithSystemMessage("open(<input_file>) failed\n");
        printBoard(serverGameboard);

        while (1){
            struct sockaddr_in6 clntAddr; // Client address
            // Set length of client address structure (in-out parameter)
            socklen_t clntAddrLen = sizeof(clntAddr);
            // Wait for a client to connect
            int sock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (sock < 0)
                DieWithSystemMessage("accept() failed");
            
            printf("client connected\n");

            HandleTCPClient(sock, &serverGameboard);
        }

    }
        
    return 0;
}



//------------------//
// SERVER FUNCTIONS //
//------------------//
void HandleTCPClient(int sock, struct action *serverGameboard){

    int BUFSIZE = sizeof(struct action);
    struct action userGameboardStatus;

    // loading board
    int numOfBombs = coutBombsOfBoard(serverGameboard->board);

    int clientCloseSocket = 0;
    do { 

        clientCloseSocket = 0;
        struct action userRequestAction;
        memset(&userRequestAction, 0, sizeof(userRequestAction));
        int numBytesRcvd = recv(sock, &userRequestAction, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        
        int x = userRequestAction.coordinates[0];
        int y = userRequestAction.coordinates[1];

        //----------------//
        // Server Actions //
        //----------------//
        if (userRequestAction.type == START_TYPE){
            // preparing board to play
            memset(&userGameboardStatus, 0, sizeof(userGameboardStatus));
            userGameboardStatus.type = STATE_TYPE;
            for (int i = 0; i < 4; i++){
                userGameboardStatus.board[i][0] = -2;
                userGameboardStatus.board[i][1] = -2;
                userGameboardStatus.board[i][2] = -2;
                userGameboardStatus.board[i][3] = -2;
            };

        }
        else if (userRequestAction.type == REVEAL_TYPE){
            // mostra celula para jogador
            userGameboardStatus.type = STATE_TYPE;
            userGameboardStatus.board[x][y] = serverGameboard->board[x][y];

            // Verifica Game Over
            if (serverGameboard->board[x][y] == BOMB_INT){
                userGameboardStatus.type = GAME_OVER_TYPE;
                for (int i = 0; i < 4; i++){
                    userGameboardStatus.board[i][0] = serverGameboard->board[i][0];
                    userGameboardStatus.board[i][1] = serverGameboard->board[i][1];
                    userGameboardStatus.board[i][2] = serverGameboard->board[i][2];
                    userGameboardStatus.board[i][3] = serverGameboard->board[i][3];
                };
            }
            else {
                // Verifica se Venceu com a atual jogada
                int numOfReveladeCells = 0;
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < 4; j++)
                        if (userGameboardStatus.board[i][j] >= 0 )
                            numOfReveladeCells++;
                
                int isWin = ((16 - numOfReveladeCells) == numOfBombs);
                
                if (isWin){
                    userGameboardStatus.type = WIN_TYPE;
                    for (int i = 0; i < 4; i++){
                        userGameboardStatus.board[i][0] = serverGameboard->board[i][0];
                        userGameboardStatus.board[i][1] = serverGameboard->board[i][1];
                        userGameboardStatus.board[i][2] = serverGameboard->board[i][2];
                        userGameboardStatus.board[i][3] = serverGameboard->board[i][3];
                    };
                }
            }
        }
        else if (userRequestAction.type == FLAG_TYPE){
            userGameboardStatus.type = STATE_TYPE;
            userGameboardStatus.board[x][y] = -3;
        }
        else if (userRequestAction.type == REMOVE_FLAG_TYPE){
            userGameboardStatus.type = STATE_TYPE;
            userGameboardStatus.board[x][y] = -2;
        }
        else if (userRequestAction.type == RESET_TYPE){
            // preparing board to play
            printf("starting new game\n");  
            memset(&userGameboardStatus, 0, sizeof(userGameboardStatus));
            userGameboardStatus.type = RESET_TYPE;
            for (int i = 0; i < 4; i++){
                userGameboardStatus.board[i][0] = -2;
                userGameboardStatus.board[i][1] = -2;
                userGameboardStatus.board[i][2] = -2;
                userGameboardStatus.board[i][3] = -2;
            };
        }
        else if (userRequestAction.type == EXIT_TYPE){
            printf("client disconnected\n");
            clientCloseSocket = 1;
        }
        

        // send board to play
        ssize_t numBytesSent = send(sock, &userGameboardStatus, sizeof(userGameboardStatus), 0);
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