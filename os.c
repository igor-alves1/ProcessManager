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

void printProcesso(Processo *p){
  if(!p) return;
  printf("#########################\nInfo do processo\n");
  printf("Tamanho (MB): %d\n", p->tamMB);
  printf("Fase 1 (CPU): %d\n", p->fase1);
  printf("Fase de I/O: %d\n", p->faseIO);
  printf("Fase 2 (CPU): %d\n#########################\n", p->fase2);
}

void printFila(Fila *f){
    Lista *l = f->first;
    if(!l) {
        printf("Fila vazia...\n");
        return;
    }
    printProcesso((Processo *) l->dado);
}

// Chamado pelo escalonador de longo prazo
void admitirProcesso(SO *so){
    if(filaVazia(so->novos)) return;

    Processo *admitido = (Processo *) removeFila(so->novos);
    addFila(so->prontos, admitido);
}

// Ainda precisa fazer a liberação da memória!
void terminarProcesso(SO *so, CPU *cpu){
    free(cpu->processo);
};

// Ainda precisa ser implementado!
int memoriaDisponivel(SO *so){
    return 1;
}

//Chamado pela Thread Geradora de Processos
void escalonadorLongoPrazo(SO *so){
    admitirProcesso(so);
}


int clockCPU(CPU *cpu){
    if(!cpu->processo) {
        return 1;
    }
    if(cpu->processo->fase1) {
        cpu->processo->fase1--;
        printf("Executando fase 1 do processo, ciclos restantes: %d\n", cpu->processo->fase1);
        cpu->time_slice++;
    }
    if(!cpu->processo->fase1){
        // Processo bloqueado para IO, gerar interrupção
        if(cpu->processo->faseIO) {
            cpu->time_slice = 0;
            printf("Processo bloqueado para fase de IO, escalonando outro processo...\n");
            return 1;
        }
        // Processo finalizado, gerar interrupção
        if(!cpu->processo->fase2){
            cpu->time_slice = 0;
            printf("Processo finalizado, escalonando outro processo...\n");
            return 1;
        }
    }
    if(!cpu->processo->fase1 && cpu->processo->fase2){
        cpu->processo->fase2--;
        printf("Executando fase 2 do processo, ciclos restantes: %d.\n", cpu->processo->fase2);
        cpu->time_slice++;
    } 
    // Processo finalizado, gerar interrupção
    if(!cpu->processo->fase1 && !cpu->processo->fase2){
        printf("Processo finalizado, escalonando outro processo...\n");
        cpu->time_slice = 0;
        return 1;
    }
    // Gerar interrupção por time-slice
    if(cpu->time_slice == 4){
        cpu->time_slice = 0;
        printf("Fim do quantum, escalonando outro processo...\n");
        return 1;
    }

    return 0;
}

// Round-robin para o escalonamento de execução
void escalonadorCurtoPrazo(SO *so, CPU *cpu){
    int run = 0;
    if(cpu->processo){
        if(cpu->processo->fase1){
            addFila(so->prontos, cpu->processo);
        }
        else if(cpu->processo->faseIO){
            addFila(so->bloqueados, cpu->processo);
        } else if(cpu->processo->fase2){
            addFila(so->prontos, cpu->processo);
        } else{
            terminarProcesso(so, cpu);
        } 
        cpu->processo = NULL;
    } else run = 1;

    if(!filaVazia(so->prontos_aux)){
        cpu->processo = (Processo *) removeFila(so->prontos_aux);
    } else if(!filaVazia(so->prontos)){
        cpu->processo = (Processo *) removeFila(so->prontos);
    }

    if(run && cpu->processo) clockCPU(cpu);
}

void clockSO(SO *so){
    if(!filaVazia(so->bloqueados)){
        Processo *b = (Processo *) peek(so->bloqueados);
        b->faseIO--;
        if(!b->faseIO){
            b = (Processo *) removeFila(so->bloqueados);
            addFila(so->prontos_aux, b);
            printf("Processo adicionado à fila auxiliar e removido da fila de bloqueados.\n");
        }
    }

    int interrupt = 0;
    for(int i=0; i < 4; i++){
        printf("Executando CPU %d: ", i+1);
        interrupt = clockCPU(so->cpus[i]);
        if(interrupt) escalonadorCurtoPrazo(so, so->cpus[i]);
        printf("\n");
    }
    printf("Fila de prontos: \n");
    printFila(so->prontos);
    printf("\n");
    
}