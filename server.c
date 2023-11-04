#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"

#define MAXPENDING 1
#define MAX_CONNECTIONS 10

//server functions
void * HandleThreadTCPClient(void * param);

//mutex
pthread_mutex_t mutex;
int ids[MAX_CONNECTIONS] = {};

// ./server v4 51511 -i input/jogo.txt
int main(int argc, char const *argv[])
{
    pthread_mutex_init(&mutex, NULL);

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

        while (1){
            struct sockaddr_in clntAddr; // Client address
            // Set length of client address structure (in-out parameter)
            socklen_t clntAddrLen = sizeof(clntAddr);
            // Wait for a client to connect
            int sock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
            if (sock < 0)
                DieWithSystemMessage("accept() failed");
            
            threadParam * param = malloc(sizeof(threadParam));
            memset(param, 0, sizeof(threadParam));
            param->sock = sock;
            param->mutex = mutex;

            pthread_t tid;
            pthread_create(&tid, NULL, HandleThreadTCPClient, param);
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
            
            threadParam * param = malloc(sizeof(threadParam));
            memset(param, 0, sizeof(threadParam));
            param->sock = sock;
            param->mutex = mutex;

            pthread_t tid;
            pthread_create(&tid, NULL, HandleThreadTCPClient, param);
        }

    }
        
    return 0;
}



//------------------//
// SERVER FUNCTIONS //
//------------------//
void * HandleThreadTCPClient(void * param){
    pthread_detach(pthread_self());

    threadParam t_param = *(threadParam *) param;
    free(param);

    BlogOperation operation;
    memset(&operation, 0, sizeof(BlogOperation));

    // give user id
    int BUFSIZE = sizeof(BlogOperation);
    int numBytesRcvd = recv(t_param.sock, &operation, BUFSIZE, 0);
    if (numBytesRcvd < 0)
        DieWithSystemMessage("recv() failed");

    pthread_mutex_lock(&mutex);
    int freeId = -1;
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
        if (ids[i] == 0){
            freeId = i;
            break;
        }
    }

    ids[freeId] = 1;
    operation.client_id = freeId;
    operation.server_response = 1;
    printf("client %d connected\n", freeId + 1);
    pthread_mutex_unlock(&mutex);
    send(t_param.sock, &operation, BUFSIZE, 0);

    int isComplete = FALSE;
    while (!isComplete){
        int numBytesRcvd = recv(t_param.sock, &operation, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        if (operation.operation_type == DESCONECTAR_SERVIDOR){
            isComplete = TRUE;
            pthread_mutex_lock(&mutex);
            ids[operation.client_id] = 0;
            pthread_mutex_unlock(&mutex);
            printf("client %d desconnected\n", operation.client_id + 1);
        }
    }
    

    

}