#include <stdio.h>
#include <sys/socket.h> //socket()
#include <netinet/in.h> //IPPROTO_TCP
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

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
