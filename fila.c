#include <stdlib.h>
#include "fila.h"
#include "TLSE.h"

Fila *inicializarFila(){
    Fila *f = (Fila *)malloc(sizeof(Fila));
    f->first = f->last = NULL;
    return f;
}

int filaVazia(Fila *f){
    if (f->first == NULL) return 1;
    return 0;
}

void addFila(Fila *f, void *dado){
    if(f->last == NULL){
        f->last = inserirLista(f->last, dado);
        f->first = f->last;
    } else {
        f->last->prox = inserirLista(f->last->prox, dado);
        f->last = f->last->prox;
    }
}

void *removeFila(Fila *f){
    if(filaVazia(f)) return NULL;
    Lista *tmp = f->first;
    void *dado = tmp->dado;
    f->first = f->first->prox;
    if(!f->first) f->last = NULL;
    free(tmp);
    return dado;
}

void *peek(Fila *f){
    if(filaVazia(f)) return NULL;
    return f->first->dado;
}