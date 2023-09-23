#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "common.h"

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
