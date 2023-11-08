#include "common.h"

#define MAXPENDING 1
#define MAX_CONNECTIONS 10

//server functions
void * HandleThreadTCPClient(void * param);

//mutex
pthread_mutex_t mutex;
int ids[MAX_CONNECTIONS] = {};
int sockets[MAX_CONNECTIONS] = {};
lista ls;


// ./server v4 51511 -i input/jogo.txt
int main(int argc, char const *argv[])
{
    pthread_mutex_init(&mutex, NULL);
    init_lista(&ls);

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
            param->mutex = &mutex;

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
            param->mutex = &mutex;

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

    int BUFSIZE = sizeof(BlogOperation);
    BlogOperation operation;
    memset(&operation, 0, sizeof(BlogOperation));


    int isComplete = FALSE;
    while (!isComplete){
        int numBytesRcvd = recv(t_param.sock, &operation, BUFSIZE, 0);
        if (numBytesRcvd < 0)
            DieWithSystemMessage("recv() failed");

        if (operation.operation_type == DESCONECTAR_SERVIDOR){
            isComplete = TRUE;

            pthread_mutex_lock(t_param.mutex);

            // desiscreve cliente de seus tópicos
            topico* iterator = ls.head;
            int i = 0;
            while (iterator != NULL){
                iterator->inscritos[operation.client_id] = FALSE;
                iterator = iterator->next;
            };
            ids[operation.client_id] = FALSE;
            printf("client %d desconnected\n", operation.client_id + 1);
            pthread_mutex_unlock(t_param.mutex);
        }
        else if(operation.operation_type == NOVA_CONEXAO) {
            int freeId = -1;
            pthread_mutex_lock(&mutex);
            for(int i = 0; i < MAX_CONNECTIONS; i++) {
                if (ids[i] == 0){
                    freeId = i;
                    break;
                }
            }
            ids[freeId] = 1;
            sockets[freeId] = t_param.sock;
            pthread_mutex_unlock(&mutex);

            operation.client_id = freeId;
            operation.server_response = TRUE;
            printf("client %d connected\n", freeId + 1);
            send(t_param.sock, &operation, BUFSIZE, 0);
        }
        else if(operation.operation_type == NOVO_POST){
            pthread_mutex_lock(t_param.mutex);
            topico *tpc = find_topico(&ls, operation.topic);

            // cria topico se não existe ainda
            if (tpc == NULL){
                tpc = malloc(sizeof(topico));
                init_topico(tpc, operation.topic);
                add_topico_lista(&ls, tpc);
            }

            // envia para todos os inscritos
            int clientID = operation.client_id;
            for (int i = 0; i < MAX_CONNECTIONS; i++){
                if (tpc->inscritos[i]){
                    operation.client_id = i;
                    operation.server_response = TRUE;
                    ssize_t numBytesSent = send(sockets[i], &operation, BUFSIZE, 0);
                    if (numBytesSent < 0)
                        DieWithSystemMessage("send() failed");
                }
            }

            operation.client_id = clientID;
            pthread_mutex_unlock(t_param.mutex);
            printf("new post added in %s by %d\n", operation.topic, operation.client_id + 1);
        }
        else if(operation.operation_type == LISTAGEM_TOPICOS){
            pthread_mutex_lock(t_param.mutex);
            topico* iterator = ls.head;
            int i = 0;
            while (iterator != NULL){
                strcpy(&operation.content[i], iterator->name);
                i += strlen(iterator->name);
                strcpy(&operation.content[i], "\n");
                i += 1;
                iterator = iterator->next;
            };
            pthread_mutex_unlock(t_param.mutex);

            operation.server_response = TRUE;
            ssize_t numBytesSent = send(t_param.sock, &operation, BUFSIZE, 0);
            if (numBytesSent < 0)
                DieWithSystemMessage("send() failed");

        }
        else if(operation.operation_type == INSCRICAO_TOPICO){

            pthread_mutex_lock(t_param.mutex);
            topico *tpc = find_topico(&ls, operation.topic);
            if (tpc == NULL){
                tpc = malloc(sizeof(topico));
                init_topico(tpc, operation.topic);
                add_topico_lista(&ls, tpc);
            }
            tpc->inscritos[operation.client_id] = 1;
            pthread_mutex_unlock(t_param.mutex);
            
            printf("client %d subscribed to %s\n", operation.client_id + 1, operation.topic);
        }
        else if(operation.operation_type == DESINSCRICAO_TOPICO){

            pthread_mutex_lock(t_param.mutex);
            topico *tpc = find_topico(&ls, operation.topic);
            if (tpc != NULL)
                tpc->inscritos[operation.client_id] = 0;
            pthread_mutex_unlock(t_param.mutex);
            printf("client %d unsubscribed to %s\n", operation.client_id + 1, operation.topic);
        }
    }
}
