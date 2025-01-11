#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include "os.h"
#include "fila.h"
#include "TLSE.h"

typedef struct appdata{
  SO *so;
  GtkApplication *app;
} AppData;

typedef struct {
  AppData *app_data;
  GtkEntry **entries;
} SubmitData;

int str_to_int(char *str){
  int l = strlen(str);
  int pow = 1, n, result = 0;

  for(int i = 0; i < l-1; i++) pow *= 10;

  for(int i=0; i < l; i++){
    n = str[i] % 48;
    result += n * pow;
    pow /= 10;
  }

  return result;
}

static void on_submit_button_clicked(GtkButton *button, gpointer user_data) {

  SubmitData *submit_data = (SubmitData *)user_data;
  AppData *app_data = submit_data->app_data;
  GtkEntry **entries = submit_data->entries;

  const char *tamanho_mb = gtk_editable_get_text(GTK_EDITABLE(entries[0]));
  const char *fase1_cpu = gtk_editable_get_text(GTK_EDITABLE(entries[1]));
  const char *fase_io = gtk_editable_get_text(GTK_EDITABLE(entries[2]));
  const char *fase2_cpu = gtk_editable_get_text(GTK_EDITABLE(entries[3]));

  if(!strcmp(tamanho_mb, "") || !strcmp(fase1_cpu, "")) {
    printf("Não é possível criar esse processo!\n");
    return;
  }

  int io, cpu2, tam, cpu1;
  tam = str_to_int(tamanho_mb);
  cpu1 = str_to_int(fase1_cpu);
  if(!strcmp(fase2_cpu, "")) cpu2 = 0;
  else cpu2 = str_to_int(fase2_cpu);
  if(!strcmp(fase_io, "")) io = 0;
  else io = str_to_int(fase_io);

  Processo *p = criarProcesso(tam, cpu1, io, cpu2);

  // Acesso à região crítica da fila de Novos
  sem_wait(app_data->so->s_mutex_novos);
  addFila(app_data->so->novos, p);
  sem_post(app_data->so->s_mutex_novos);
  sem_post(app_data->so->s_empty_novos);

  g_print("#########################\nNovo processo criado!\n");
  g_print("Tamanho (MB): %d\n", tam);
  g_print("Fase 1 (CPU): %d\n", cpu1);
  g_print("Fase de I/O: %d\n", io);
  g_print("Fase 2 (CPU): %d\n#########################\n", cpu2);
}

static void activate (GtkApplication *app, gpointer user_data){
  GtkWidget *window;

  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Submeta um processo:");

  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
  gtk_widget_set_margin_top(grid, 20);
  gtk_widget_set_margin_bottom(grid, 20);
  gtk_widget_set_margin_start(grid, 20);
  gtk_widget_set_margin_end(grid, 20);

  gtk_window_set_child(GTK_WINDOW(window), grid);

  // Criação dos widgets
  GtkWidget *label_tamanho = gtk_label_new("Tamanho (MB):");
  GtkWidget *entry_tamanho = gtk_entry_new();

  GtkWidget *label_fase1 = gtk_label_new("Fase 1 (CPU):");
  GtkWidget *entry_fase1 = gtk_entry_new();

  GtkWidget *label_fase_io = gtk_label_new("Fase de I/O:");
  GtkWidget *entry_fase_io = gtk_entry_new();

  GtkWidget *label_fase2 = gtk_label_new("Fase 2 (CPU):");
  GtkWidget *entry_fase2 = gtk_entry_new();

  GtkWidget *button_submit = gtk_button_new_with_label("Enviar");
  GtkEntry **entries = g_malloc(sizeof(GtkEntry *) * 4);
  entries[0] = GTK_ENTRY(entry_tamanho);
  entries[1] = GTK_ENTRY(entry_fase1);
  entries[2] = GTK_ENTRY(entry_fase_io);
  entries[3] = GTK_ENTRY(entry_fase2);
  SubmitData *submit_data = g_malloc(sizeof(SubmitData));
  submit_data->app_data = user_data;
  submit_data->entries = entries;

  g_signal_connect(button_submit, "clicked", G_CALLBACK(on_submit_button_clicked), submit_data);

  // Adicionando widgets ao grid
  gtk_grid_attach(GTK_GRID(grid), label_tamanho, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), entry_tamanho, 1, 0, 1, 1);

  gtk_grid_attach(GTK_GRID(grid), label_fase1, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), entry_fase1, 1, 1, 1, 1);

  gtk_grid_attach(GTK_GRID(grid), label_fase_io, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), entry_fase_io, 1, 2, 1, 1);

  gtk_grid_attach(GTK_GRID(grid), label_fase2, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), entry_fase2, 1, 3, 1, 1);

  gtk_grid_attach(GTK_GRID(grid), button_submit, 0, 4, 2, 1);

  gtk_window_present (GTK_WINDOW (window));
  g_signal_connect(window, "destroy", G_CALLBACK(g_free), entries);
}

void *thread_UI(void *arg){
  AppData *data = (AppData *)arg;
  int *status = (int *)malloc(sizeof(int));

  g_signal_connect(data->app, "activate", G_CALLBACK (activate), data);
  (*status) = g_application_run(G_APPLICATION(data->app), 0, NULL);
  g_object_unref(data->app);
  pthread_exit(status);
}

// Consome da fila de Novos e produz para a fila de Prontos
void *thread_LongoPrazo(void *arg){
  AppData *data = (AppData *) arg;
  SO *so = data->so;

  int i = 0;
  while(data->app){
    if(memoriaDisponivel(so)){
      sem_wait(so->s_empty_novos);
      sem_wait(so->s_mutex_novos);
      escalonadorLongoPrazo(so);
      sem_post(so->s_mutex_novos);
    }
  }
  pthread_exit(NULL);
}

//Consome da fila de Prontos e executa Processos
void *thread_execucao(void *arg){
  AppData *data = (AppData *) arg;
  SO *so = data->so;

  sleep(5);
  while(data->app){
    sleep(3);
    clockSO(so);
  }
  pthread_exit(NULL);
}

int main (int argc, char **argv){
  pthread_t thread_ui, thread_longterm, thread_cpu;
  void *status, *thread_exec;

  AppData *data = malloc(sizeof(AppData));
  data->app = gtk_application_new ("org.gtk.example", 0);
  data->so = inicializarSO();

  pthread_create(&thread_ui, NULL, thread_UI, data);
  pthread_create(&thread_longterm, NULL, thread_LongoPrazo, data);
  pthread_create(&thread_cpu, NULL, thread_execucao, data);

  pthread_join(thread_ui, &status);
  pthread_join(thread_longterm, &thread_exec);
  pthread_join(thread_cpu, &thread_exec);

  int *stat = (int *)status;
  int res = *stat;
  free(stat);

  return res;
}
