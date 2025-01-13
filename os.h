
#pragma once
#include <stdlib.h>
#include <semaphore.h>
#include "TLSE.h"
#include "fila.h"

#define QUANTUM 4
#define MEM_SIZE 32768 // tam do array da MP em MB
#define ARRAY_SIZE MEM_SIZE/8
#define PAGE_SIZE 1 // tam da pag em MB

typedef struct Process {
    int id;
    int fase1, faseIO, fase2;
    int tamMB;
    int *end_mp;
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
    sem_t *s_empty_novos;
    sem_t *s_mutex_novos;
    u_int8_t *RAM;
    int num_processos;
    int interrupt;
    
} SO;

SO *inicializarSO();
Processo *criarProcesso(int tam, int fase1, int faseIO, int fase2);
void escalonadorLongoPrazo(SO *so);
int memoriaDisponivel(SO *so);
void clockSO(SO *so);