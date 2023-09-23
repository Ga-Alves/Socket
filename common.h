#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

struct message {
    int type;
    int coordinates[2];
    int board[4][4];
};
void DieWithUserMessage(const char *msg, const char *detail) ;
void DieWithSystemMessage(const char *msg);
void HandleTCPServer(int clntSock);
void HandleTCPClient(int clntSock, const char* gamePath);
int initGameBoard(struct message *serverGameBoard, const char* gamePath);
void printBoard(struct message message);
