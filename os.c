#include <semaphore.h>
#include <stdio.h>
#include "fila.h"
#include "os.h"

SO *inicializarSO(){
    SO *so = (SO *)malloc(sizeof(SO));
    so->bloqueados = inicializarFila();
    so->prontos = inicializarFila();
    so->prontos_aux = inicializarFila();
    so->novos = inicializarFila();
    so->RAM = inicializarLista();
    for(int i=0; i<4; i++) {
        so->cpus[i] = (CPU *)malloc(sizeof(CPU));
        so->cpus[i]->processo = NULL;
        so->cpus[i]->time_slice = 0;
    }

    so->s_empty_novos = (sem_t *)malloc(sizeof(sem_t));
    so->s_empty_prontos = (sem_t *)malloc(sizeof(sem_t));
    so->s_mutex_novos = (sem_t *)malloc(sizeof(sem_t));
    so->s_mutex_prontos = (sem_t *)malloc(sizeof(sem_t));
    sem_init(so->s_empty_novos, 0, 0);
    sem_init(so->s_empty_prontos, 0, 0);
    sem_init(so->s_mutex_novos, 0, 1);
    sem_init(so->s_mutex_prontos, 0, 1);
    return so;
}

Processo *criarProcesso(int tam, int fase1, int faseIO, int fase2){
    Processo *p = (Processo *)malloc(sizeof(Processo));
    p->fase1 = fase1;
    p->fase2 = fase2;
    p->faseIO = faseIO;
    p->tamMB = tam;
    return p;
}

// Chamado pelo escalonador de longo prazo
void admitirProcesso(SO *so){
    if(filaVazia(so->novos)) return;

    Processo *admitido = (Processo *) removeFila(so->novos);
    addFila(so->prontos, admitido);
}

//Chamado pela Thread Geradora de Processos
void escalonadorLongoPrazo(SO *so){
}

// Round-robin para o escalonamento de execução
void escalonadorCurtoPrazo(SO *so, CPU *cpu){
    if(!filaVazia(so->prontos_aux)){
        cpu->processo = (Processo *) removeFila(so->prontos_aux);
    } else if(!filaVazia(so->prontos)){
        cpu->processo = (Processo *) removeFila(so->prontos);
    } 
}

int clockCPU(CPU *cpu){
    // Idle
    if(!cpu->processo) return 1;
    if(cpu->processo->fase1) cpu->processo->fase1--;
    else if(cpu->processo->fase2) cpu->processo->fase2--; 

    // Release
    if(!cpu->processo->fase1 && !cpu->processo->fase2 && !cpu->processo->faseIO){
        free(cpu->processo);
        cpu->processo = NULL;
        return 1;
    }
    // Block
    if(!cpu->processo->fase1) return 1;
    return 0;
}