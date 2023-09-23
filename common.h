#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>

#define START_TYPE 0
#define REVEAL_TYPE 1
#define FLAG_TYPE 2
#define STATE_TYPE 3
#define REMOVE_FLAG_TYPE 4
#define RESET_TYPE 5
#define WIN_TYPE 6
#define EXIT_TYPE 7
#define GAME_OVER_TYPE 8

struct message {
    int type;
    int coordinates[2];
    int board[4][4];
};
void DieWithUserMessage(const char *msg, const char *detail) ;
void DieWithSystemMessage(const char *msg);

//server functions
void HandleTCPClient(int clntSock, const char* gamePath);
int initGameBoard(struct message *serverGameBoard, const char* gamePath);

//client functions
void HandleTCPServer(int serverSock);
void printBoard(struct message message);
