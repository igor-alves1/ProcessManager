#include <semaphore.h>
#include <stdio.h>
#include "fila.h"
#include "os.h"




void printProcessBanner() {
    // Cada linha termina com \n e precisa manter a mesma formatação
    // do ASCII que você forneceu.
    printf("______                                     ___  ___                                        \n");
    printf("| ___ \\                                    |  \\/  |                                        \n");
    printf("| |_/ / _ __   ___    ___   ___  ___  ___  | .  . |  __ _  _ __    __ _   __ _   ___  _ __ \n");
    printf("|  __/ | '__| / _ \\  / __| / _ \\/ __|/ __| | |\\/| | / _` || '_ \\  / _` | / _` | / _ \\| '__|\n");
    printf("| |    | |   | (_) || (__ |  __/\\__ \\\\__ \\ | |  | || (_| || | | || (_| || (_| ||  __/| |   \n");
    printf("\\_|    |_|    \\___/  \\___| \\___||___/|___/ \\_|  |_/ \\__,_||_| |_| \\__,_| \\__, | \\___||_|   \n");
    printf("                                                                          __/ |               \n");
    printf("                                                                         |___/                \n");
}



SO *inicializarSO(){
    SO *so = (SO *)malloc(sizeof(SO));
    so->num_processos = 0;
    so->interrupt = 0;
    so->bloqueados = inicializarFila();
    so->prontos = inicializarFila();
    so->prontos_aux = inicializarFila();
    so->novos = inicializarFila();
    so->RAM = calloc(ARRAY_SIZE, sizeof(u_int8_t));
    for(int i=0; i<4; i++) {
        so->cpus[i] = (CPU *)malloc(sizeof(CPU));
        so->cpus[i]->processo = NULL;
        so->cpus[i]->time_slice = 0;
    }

    so->s_empty_novos = (sem_t *)malloc(sizeof(sem_t));
    so->s_mutex_novos = (sem_t *)malloc(sizeof(sem_t));
    sem_init(so->s_empty_novos, 0, 0);
    sem_init(so->s_mutex_novos, 0, 1);
    return so;
}

Processo *criarProcesso(int tam, int fase1, int faseIO, int fase2){
    Processo *p = (Processo *)malloc(sizeof(Processo));
    p->tamMB = tam;
    p->end_mp = (int*)malloc(sizeof(int) * p->tamMB/PAGE_SIZE);
    p->fase1 = fase1;
    p->fase2 = fase2;
    p->faseIO = faseIO;
    

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
    while(l){
        Processo *p = (Processo *) l->dado;   
        printf("-> P%d (%d) ", p->id, p->faseIO);
        l = l->prox;
    }
    printf("\n");
}


// Chamado pelo escalonador de longo prazo
void admitirProcesso(SO *so){
    if(filaVazia(so->novos)) return;

    Processo *admitido = (Processo *) removeFila(so->novos);
    admitido->id = ++so->num_processos;
    addFila(so->prontos, admitido);
}

// Ainda precisa fazer a liberação da memória!
void terminarProcesso(SO *so, CPU *cpu){
    for(int i = 0; i < cpu->processo->tamMB; i++){
        int ind = cpu->processo->end_mp[i]/8, des = cpu->processo->end_mp[i] % 8;
        so->RAM[ind] = so->RAM[ind] ^ (1<<des);
    }
    free(cpu->processo->end_mp);
    free(cpu->processo);
    so->interrupt = 1;
}

// Ainda precisa ser implementado!
int memoriaDisponivel(SO *so){
    Processo *p = peek(so->novos);
    int c = 0;
    for (int i = 0; i < ARRAY_SIZE; i++){
        for(int j = 7; j >= 0; j--){
            if(!(so->RAM[i] & (1<<j))){
                c++;
                if(c >= p->tamMB) return 1;
            } 
        }
    }
    return 0;
}

//Chamado pela Thread Geradora de Processos
void escalonadorLongoPrazo(SO *so){
    Processo *p = peek(so->novos);
    int c = p->tamMB;
    int g = 0;
    for (int i = 0; i < ARRAY_SIZE; i++){
        if (g) break;
        for(int j = 7; j >= 0; j--){
            if(!(so->RAM[i] & (1<<j))){ //verifica se o bit está vazio
                so->RAM[i] = so->RAM[i] | (1<<j); //aloca nesse bit
                p->end_mp[p->tamMB-c] = i*8 + j;
                c--;
                if(c == 0) {
                    g = 1;
                    break;
                }
            } 
        }
    }
    admitirProcesso(so);


}


int clockCPU(CPU *cpu){
    if(!cpu->processo) {
        return 1;
    }
    if(cpu->processo->fase1) {
        cpu->processo->fase1--;
        printf("Executando fase 1 do processo P%d, ciclos restantes: %d\n", cpu->processo->id, cpu->processo->fase1);
        cpu->time_slice++;
    }
    if(!cpu->processo->fase1){
        // Processo bloqueado para IO, gerar interrupção
        if(cpu->processo->faseIO) {
            cpu->time_slice = 0;
            printf("Processo P%d bloqueado para fase de IO, escalonando outro processo...\n", cpu->processo->id);
            return 1;
        }
        // Processo finalizado, gerar interrupção
        if(!cpu->processo->fase2){
            cpu->time_slice = 0;
            printf("Processo P%d finalizado, escalonando outro processo...\n", cpu->processo->id);
            return 1;
        }
    }
    if(!cpu->processo->fase1 && cpu->processo->fase2){
        cpu->processo->fase2--;
        printf("Executando fase 2 do processo P%d, ciclos restantes: %d.\n", cpu->processo->id, cpu->processo->fase2);
        cpu->time_slice++;
    } 
    // Processo finalizado, gerar interrupção
    if(!cpu->processo->fase1 && !cpu->processo->fase2){
        printf("Processo P%d finalizado, escalonando outro processo...\n", cpu->processo->id);
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
    } else {
        so->interrupt = 1;
        run = 1;
    }

    if(!filaVazia(so->prontos_aux)){
        cpu->processo = (Processo *) removeFila(so->prontos_aux);
    } else if(!filaVazia(so->prontos)){
        cpu->processo = (Processo *) removeFila(so->prontos);
    }

    if(run && cpu->processo) clockCPU(cpu);
}

void clockSO(SO *so){
    // Limpa a tela e printa o banner
    fflush(stdout);
    system("clear");
    printProcessBanner();

    if(!filaVazia(so->bloqueados)){
        Processo *b = (Processo *) peek(so->bloqueados);
        b->faseIO--;
        if(!b->faseIO){
            b = (Processo *) removeFila(so->bloqueados);
            addFila(so->prontos_aux, b);
            printf("Processo P%d adicionado à fila auxiliar e removido da fila de bloqueados.\n", b->id);
        }
    }

    int interrupt = 0;
    for(int i=0; i < 4; i++){
        printf("Executando CPU %d: ", i+1);
        interrupt = clockCPU(so->cpus[i]);
        if(interrupt) escalonadorCurtoPrazo(so, so->cpus[i]);
        printf("\n\n");
    }
    printf("Fila de prontos: ");
    printFila(so->prontos);
    printf("Fila de bloqueados: ");
    printFila(so->bloqueados);
    printf("\n");
    //printMemoria(so->RAM);
}