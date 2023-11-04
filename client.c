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

    BlogOperation operation;
    memset(&operation, 0, sizeof(BlogOperation));
    scanf("%s", operation.content);

    ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
    if (numBytesSent < 0)
        DieWithSystemMessage("send() failed");

    int errorclose = close(sock);
    if (errorclose < 0)
        DieWithSystemMessage("close() failed");
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
