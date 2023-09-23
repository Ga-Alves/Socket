#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>

#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>


void DieWithUserMessage(const char *msg, const char *detail);
void DieWithSystemMessage(const char *msg);

struct action {
    int type;
    int coordinates[2];
    int board[4][4];
};

void HandleTCPServer(int clntSock);
void printBoard(struct action message);

int main(int argc, char const *argv[])
{

    if (argc < 3){
        printf("Quantidade de parâmetros errada!\n");
        return -1;
    }
    
    const char* IP = argv[1];
    const char* PORT = argv[2];

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
    
    return 0;
}


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
    int BUFSIZE = sizeof(struct action);
    struct action message;
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
void printBoard(struct action message){
    for (int i = 0; i < 4; i++){
        printf("%d, %d, %d, %d\n", message.board[i][0], message.board[i][1], message.board[i][2], message.board[i][3] );
    };
}