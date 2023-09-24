#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define MAXPENDING 1

// ./server v4 51511 -i input/jogo.txt
int main(int argc, char const *argv[])
{

    if (argc < 5){
        printf("Quantidade de parâmetros errada!\n");
        return -1;
    }

    in_port_t servPort = atoi(argv[2]); // First arg: local port

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
            int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (clntSock < 0)
                DieWithSystemMessage("accept() failed");
            
            printf("client connected\n");

            HandleTCPClient(clntSock, argv[4]);
        }

    }
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
            int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (clntSock < 0)
                DieWithSystemMessage("accept() failed");
            
            printf("client connected\n");

            HandleTCPClient(clntSock, argv[4]);
        }

    }
        
    return 0;
}
