#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

// tipos de operação
#define NOVA_CONEXAO 1
#define NOVO_POST 2
#define LISTAGEM_TOPICOS 3
#define INSCRICAO_TOPICO 4
#define DESCONECTAR_SERVIDOR 5
#define DESINSCRICAO_TOPICO 6

#define TRUE 1
#define FALSE 0

struct BlogOperation {
    int client_id;
    int operation_type;
    int server_response;
    char topic[50];
    char content[2048];
} typedef BlogOperation;

struct threadParam {
    pthread_mutex_t mutex;
    int sock;
} typedef threadParam;


void DieWithUserMessage(const char *msg, const char *detail) ;
void DieWithSystemMessage(const char *msg);
char* substring(int init, int end, char string[1000]);
