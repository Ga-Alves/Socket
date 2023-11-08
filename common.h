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
    pthread_mutex_t *mutex;
    int sock;
} typedef threadParam;

struct topico{
    char name[50];
    int inscritos[10];
    struct topico *next;
} typedef topico;

struct lista{
    topico *head;
    topico *tail;

}typedef lista;

// topico methods
void init_topico(topico* tpc, char name[50]);
void subscribe_user_on_topico(int userID, topico* tpc);
void unsubscribe_user_on_topico(int userID, topico* tpc);
void init_lista(lista* ls);

//lista methods
void add_topico_lista(lista* ls, topico *tpc);
topico* find_topico(lista* ls, char topicName[50]);


void DieWithUserMessage(const char *msg, const char *detail) ;
void DieWithSystemMessage(const char *msg);
char* substring(int init, int end, char string[1000]);
