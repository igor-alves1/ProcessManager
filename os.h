
#pragma once
#include <stdlib.h>
#include <semaphore.h>
#include "TLSE.h"
#include "fila.h"

#define QUANTUM 4

typedef struct Process {
    int id;
    int fase1, faseIO, fase2;
    int tamMB;
} Processo;

typedef struct cpu {
    int time_slice;
    Processo *processo;
} CPU;

typedef struct OS {
    CPU *cpus[4];
    Fila *prontos;
    Fila *prontos_aux;
    Fila *bloqueados;
    Fila *novos;
    Lista *RAM;
    sem_t *s_empty_novos;
    sem_t *s_mutex_novos;
    sem_t *s_empty_prontos;
    sem_t *s_mutex_prontos;
} SO;

SO *inicializarSO();
Processo *criarProcesso(int tam, int fase1, int faseIO, int fase2);
void escalonadorLongoPrazo(SO *so);
void escalonadorCurtoPrazo(SO *so, CPU *cpu);