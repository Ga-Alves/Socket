#include "common.h"

//client functions
void HandleTCPServer(int sock);
int isIPv4(const char *ipAddress);
int isIPv6(const char *ipAddress);
void * printServerResponseThread(void * param);

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

    int myID = 0;
    BlogOperation operation;

    // primeira requisição
    memset(&operation, 0, sizeof(BlogOperation));
    operation.client_id = 0;
    operation.operation_type = NOVA_CONEXAO;
    operation.server_response = FALSE;
    strcpy(operation.topic, "");
    strcpy(operation.content, "");

    ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
    if (numBytesSent < 0)
        DieWithSystemMessage("send() failed");

    //recebe client id
    int numBytesRcvd = recv(sock, &operation, sizeof(BlogOperation), 0);
    if (numBytesRcvd < 0)
        DieWithSystemMessage("recv() failed");
    
    myID = operation.client_id;

    pthread_t tid;
    pthread_create(&tid, NULL, printServerResponseThread, &sock);

    int isExit = 0;
    while (!isExit){
        char inptStr[1000] = {};
        fgets(inptStr, sizeof(inptStr), stdin);

        char *first4 = substring(0, 4, inptStr);
        char *first9 = substring(0, 9, inptStr);
        char *first10 = substring(0, 10, inptStr);
        char *first11 = substring(0, 11, inptStr);

        int exit = !strcmp(first4, "exit");
        int subscribe = !strcmp(first9, "subscribe");
        int publish_in = !strcmp(first10, "publish in");
        int list = !strcmp(first11, "list topics");
        int unsubscribe = !strcmp(first11, "unsubscribe");

        free(first4);
        free(first9);
        free(first10);
        free(first11);


        if (exit){
            // printf("comando exit reconhecido!\n");
            memset(&operation, 0, sizeof(BlogOperation));
            operation.server_response = FALSE;
            operation.operation_type = DESCONECTAR_SERVIDOR;
            operation.client_id = myID;

            ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");

            isExit = TRUE;
        }
        else if (list){
            memset(&operation, 0, sizeof(BlogOperation));
            operation.client_id = myID;
            operation.operation_type = LISTAGEM_TOPICOS;
            operation.server_response = FALSE;
            ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");
        }
        else if (subscribe){
            char subscribe[50] = {};
            char topico[50] = {};
            sscanf(inptStr, "%s %s", subscribe, topico);

            memset(&operation, 0, sizeof(BlogOperation));
            operation.client_id = myID;
            operation.server_response = FALSE;
            operation.operation_type = INSCRICAO_TOPICO;
            strcpy(operation.topic, topico);
            ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");

        }
        else if (publish_in){
            // printf("comando publish_in reconhecido!\n");
            char publish[50] = {};
            char in[50] = {};
            char topico[50] = {};
            sscanf(inptStr, "%s %s %s", publish, in, topico);


            memset(&operation, 0, sizeof(BlogOperation));
            operation.client_id = myID;
            operation.server_response = FALSE;
            operation.operation_type = NOVO_POST;
            strcpy(operation.topic, topico);
            fgets(operation.content, sizeof(operation.content), stdin);
            
            ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");

        }
        else if (unsubscribe){
            char unsubscribe[50] = {};
            char topico[50] = {};
            sscanf(inptStr, "%s %s", unsubscribe, topico);


            memset(&operation, 0, sizeof(BlogOperation));
            operation.client_id = myID;
            operation.server_response = FALSE;
            operation.operation_type = DESINSCRICAO_TOPICO;
            strcpy(operation.topic, topico);
            ssize_t numBytesSent = send(sock, &operation, sizeof(BlogOperation), 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");
        }
        else{
            printf("COMMAND NOT FOUND\n");
        }
        
    }
    
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
void * printServerResponseThread(void * param){
    pthread_detach(pthread_self());

    int sock = *(int*)param;
    while (TRUE){
        BlogOperation operation;
        int numBytesRcvd = recv(sock, &operation, sizeof(BlogOperation), 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");
        
        if (operation.operation_type == NOVO_POST){
            // adiciona informações da mensagem
            char header[80] = "new post added in ";
            strcat(header, operation.topic);
            strcat(header, " by ");
            char id[2] = {};
            snprintf(id, sizeof(id), "%d", operation.client_id);
            strcat(header, id);
            strcat(header, "\n");
            strcat(header, operation.content);

            strcpy(operation.content, header);
        }
        
        printf("%s", operation.content);

    }

}