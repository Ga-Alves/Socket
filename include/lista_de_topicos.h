#ifndef LISTA_TOPICOS
#define LISTA_TOPICOS

struct topico{
    char name[50];
    int inscritos[10];
    struct topico *next;
} typedef topico;

struct lista{
    topico *head;
    topico *tail;

}typedef lista;

// topicos
void init_topico(topico* tpc, char name[50]);
void subscribe_user_on_topico(int userID, topico* tpc);
void unsubscribe_user_on_topico(int userID, topico* tpc);

//lista de topicos
void init_lista(lista* ls);
void add_topico_lista(lista* ls, topico *tpc);
topico* find_topico(lista* ls, char topicName[50]);

#endif