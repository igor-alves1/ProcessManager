#pragma once

#include <stdlib.h>
#include "fila.h"
#include "TLSE.h"

typedef struct Fila{
    Lista *first;
    Lista *last;
} Fila;

Fila *inicializarFila();
int filaVazia(Fila *f);
void addFila(Fila *f, void *dado);
void *removeFila(Fila *f);
void *peek(Fila *f);