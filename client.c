#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "common.h"

//client functions
void HandleTCPServer(int sock);
void printBoard(struct action message);
int isIPv4(const char *ipAddress);
int isIPv6(const char *ipAddress);

int main(int argc, char const *argv[])
{

    if (argc < 3){
        printf("Quantidade de parâmetros errada!\n");
        return -1;
    }
    
    const char* IP = argv[1];
    const char* PORT = argv[2];

    if (isIPv4(argv[1])){
        in_port_t servPort = atoi(PORT);

        int sock= socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0)
        {
            printf("Erro em socket()\n");
            return -1;
        };


        // Server address struct
        struct sockaddr_in servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;

        int rtnVal = inet_pton(AF_INET, IP, &servAddr.sin_addr.s_addr);
        if (rtnVal == 0)
            DieWithUserMessage("inet_pton() failed", "invalid address string");
        else if (rtnVal < 0)
            DieWithSystemMessage("inet_pton() failed");
        servAddr.sin_port = htons(servPort);


        // conectando
        if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
            DieWithSystemMessage("connect() failed");
        
        HandleTCPServer(sock);
    }
    else if(isIPv6(argv[1])){
        in_port_t servPort = atoi(PORT);

        int sock= socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0)
        {
            printf("Erro em socket()\n");
            return -1;
        };


        // Server address struct
        struct sockaddr_in6 servAddr;
        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin6_family = AF_INET6;

        int rtnVal = inet_pton(AF_INET6, IP, &servAddr.sin6_addr);
        if (rtnVal == 0)
            DieWithUserMessage("inet_pton() failed", "invalid address string");
        else if (rtnVal < 0)
            DieWithSystemMessage("inet_pton() failed");
        servAddr.sin6_port = htons(servPort);


        // conectando
        if (connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
            DieWithSystemMessage("connect() failed");
        
        HandleTCPServer(sock);
    }
    else {
        DieWithSystemMessage("Unknow IP version!");
    }
    

    
    return 0;
}


//------------------//
// CLIENT FUNCTIONS //
//------------------//
void HandleTCPServer(int sock){
    while (1){
        int BUFSIZE = sizeof(struct action);
        struct action message;
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
            ssize_t numBytesSent = send(sock, &message, sizeof(message), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");
            int errorClose = close(sock);
            if (errorClose < 0)
                DieWithSystemMessage("close() failed");
            return;
        }
        
        // init game
        message.type = START_TYPE;
        ssize_t numBytesSent = send(sock, &message, sizeof(message), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");
        
        
        ssize_t numBytesRcvd = recv(sock, &message, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        printBoard(message);

        int isGameFinish = 0;
        do{
            isGameFinish = 0;
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
                ssize_t numBytesSent = send(sock, &message, sizeof(message), 0);
                if (numBytesSent < 0)
                    DieWithSystemMessage("send() failed");
                int errorclose = close(sock);
                if (errorclose < 0)
                    DieWithSystemMessage("close() failed");
                return;
            }
            else {
                printf("error: command not found\n");
                hasError = 1;
            }
            
            // handle erros on client side
            if (!hasError){
                ssize_t numBytesSent = send(sock, &message, sizeof(message), 0);
                if (numBytesSent < 0)
                    DieWithSystemMessage("send() failed");
                
                
                ssize_t numBytesRcvd = recv(sock, &message, BUFSIZE, 0);
                if (numBytesRcvd < 0)
                    DieWithSystemMessage("recv() failed");

                if(message.type == GAME_OVER_TYPE){
                    isGameFinish = 1;
                    printf("GAME OVER!\n");
                }
                else if(message.type == WIN_TYPE){
                    isGameFinish = 1;
                    printf("YOU WIN!\n");
                }
                printBoard(message);
            }
            
        }while(!isGameFinish);   
    }
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
int isIPv4(const char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));
    return result != 0;
}
int isIPv6(const char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET6, ipAddress, &(sa.sin_addr));
    return result != 0;
}
