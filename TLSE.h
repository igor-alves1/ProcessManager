#pragma once

#include <stdlib.h>
#include <string.h>

typedef struct No {
    void *dado;
    struct No *prox;
} Lista;

Lista *inicializarLista();
Lista *inserirLista(Lista *l, void *dado);