#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXPENDING 1


void DieWithUserMessage(const char *msg, const char *detail);
void DieWithSystemMessage(const char *msg);
void HandleTCPClient(int clntSock);

// ./server v4 51511 -i input/jogo.txt
int main(int argc, char const *argv[])
{

    if (argc < 5){
        printf("Quantidade de parâmetros errada!\n");
        return -1;
    }

    in_port_t servPort = atoi(argv[2]); // First arg: local port

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

    // Bind to the local address
    if (bind(servSock, (struct sockaddr*) &servAddr, sizeof(servAddr)) < 0)
        DieWithSystemMessage("bind() failed");

    // linten channel
    if (listen(servSock, MAXPENDING) < 0)
        DieWithSystemMessage("listen() failed");


    
    struct sockaddr_in clntAddr; // Client address
    // Set length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);
    // Wait for a client to connect
    int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
        DieWithSystemMessage("accept() failed");
    

    // Print client Address
    char clntName[INET_ADDRSTRLEN]; // String to contain client address
    if (inet_ntop(AF_INET, &clntAddr.sin_addr.s_addr, clntName, INET_ADDRSTRLEN))
        printf("Handling client %s/%d\n", clntName, ntohs(clntAddr.sin_port));
    else
        puts("Unable to get client address");


    HandleTCPClient(clntSock);
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

void HandleTCPClient(int clntSock){
    struct action {
        int type;
        int coordinates[2];
        int board[4][4];
    };

    int BUFSIZE = sizeof(struct action);

   do { 
        struct action serverResponse;
        serverResponse.type = 777;
        ssize_t numBytesSent = send(clntSock, &serverResponse, sizeof(serverResponse), 0);
        if (numBytesSent < 0)
            DieWithSystemMessage("send() failed");

        
        // // See if there is more data to receive
        // numBytesRcvd = recv(clntSock, &userAction, BUFSIZE, 0);
        // if (numBytesRcvd < 0)
        //     DieWithSystemMessage("recv() failed");
    }  while (0);
    

}