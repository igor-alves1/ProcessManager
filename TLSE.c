#include <stdlib.h>
#include <string.h>
#include "TLSE.h"

Lista *inicializarLista(){
    return NULL;
}

Lista *inserirLista(Lista *l, void *dado){
    Lista *novo = (Lista *)malloc(sizeof(Lista));

    novo->dado = dado;
    novo->prox = l;

    return novo;
}