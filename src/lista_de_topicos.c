#include "lista_de_topicos.h"
#include <stdio.h>
#include <string.h>

// TOPICO
void init_topico(topico* tpc, char name[50]){
    memset(tpc, 0, sizeof(topico));
    strcpy(tpc->name, name);
    
    for (int i = 0; i < 10; i++)
        tpc->inscritos[i] = 0;
    
    tpc->next = NULL;
}

void subscribe_user_on_topico(int userID, topico* tpc){
    tpc->inscritos[userID - 1] = 1;
}
void unsubscribe_user_on_topico(int userID, topico* tpc){
    tpc->inscritos[userID - 1] = 0;
}

//LISTA
void init_lista(lista* ls){
    memset(ls, 0, sizeof(lista));
    ls->head = NULL;
    ls->tail = NULL;
}

void add_topico_lista(lista* ls, topico *tpc){

    if (ls->head == NULL && ls->tail == NULL){
        ls->head = tpc;
        ls->tail = tpc;
    }
    else {
        ls->tail->next = tpc;
        ls->tail = tpc;
    }
}

topico* find_topico(lista* ls, char topicName[50]){
    topico* iterator = ls->head;


    int isFind = 0;
    while (iterator != NULL){
        isFind = !strcmp(iterator->name, topicName);
        if (isFind)
            return iterator;
        iterator = iterator->next;
    };

    return NULL;
}
