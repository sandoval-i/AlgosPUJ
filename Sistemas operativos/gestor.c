#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "util.h"

bool* connected;
char* pipe;

void load_file(const char* filename, int N, bool** adjMat) { // Lee el archivo que contiene el grafo de relaciones iniciales
  int i,j, temp, e;
  FILE* fp = fopen(filename, "r");

  if(!fp) {
    perror("Error abriendo el archivo de relaciones: ");
    exit(1);
  }

  for(i = 0; i < N; ++i)
    for(j = 0; j < N; ++j) {
      e = fscanf(fp, "%d", &temp);
      if(e == EOF) {
        perror("Error leyendo el archivo de relaciones: ");
        exit(1);
      }
      adjMat[i][j] = (temp == 1);
    }
}

bool** reservar_matriz_booleanos(int rows, int columns) { // Reserva espacio para una matriz de booleanos de rows * columns
  int i;
  bool** mat = malloc(rows * sizeof(bool*));;

  if(!mat) {
    perror("Error reservando memoria para una matriz de booleanos: ");
    exit(1);
  }

  for(i = 0; i < rows; ++i) {
    mat[i] = malloc(columns * sizeof(bool));
    if(!mat) {
      perror("Error reservando memoria para una matriz de booleanos: ");
      exit(1);
    }
  }

  return mat;
}

bool* reservar_arreglo_booleanos(int rows) {
  bool* arr = malloc(rows * sizeof(bool));

  if(!arr) {
    perror("Error reservando memoria para un arreglo de booleanos: ");
    exit(1);
  }

  return arr;
}

void crear_pipe(const char* pipename) {
  unlink(pipename);
  printf("Creando un pipe con nombre \"%s\"\n", pipename);
  bool creado = false;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  while(!creado)
    if(mkfifo(pipename, fifo_mode) == -1) {
      perror("No se puede crear el pipe: ");
      printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
      sleep(SLEEP_TIME);
    } else  creado = true;
    printf("Pipe con nombre \"%s\" creado\n", pipename);
}

void registrar(const char* pipename, int users) {
  int id, fd;
  bool registrado = true;

  fd = open(pipename, O_RDONLY);
  read(fd, &id, sizeof(int));
  close(fd);

  if(--id >= 0 && id < users) {
    registrado = connected[id];
    connected[id] = true;
  }

  fd = open(pipename, O_WRONLY);
  write(fd, &registrado, sizeof(bool));
  close(fd);

}

void reg(const char* pipename, pid_t pid, int users) {
  int fd = open(pipename, O_WRONLY);

  write(fd, &pid, sizeof(pid_t));
  close(fd);

  registrar(pipename, users);

}

void log_out() {
  int fd, id;
  
  fd = open(pipe, O_RDONLY);
  read(fd, &id, sizeof(int));
  close(fd);

  connected[--id] = false;
}

int main(int argc, char* argv[]) {
  if(argc != 5) {
    printf("Error, usage: %s N relaciones modo pipeNom\n", argv[0]);
    exit(1);
  }

  int users = atoi(argv[1]), modo = atoi(argv[3]), fd;
  const char* filename = strdup(argv[2]);
  pipe = strdup(argv[4]);
  bool** adjMat = reservar_matriz_booleanos(users, users);
  connected = reservar_arreglo_booleanos(users);
  pid_t pid = getpid();

  // Inicialmente no hay ningun cliente conectado
  memset(connected, false, sizeof connected);

  load_file(filename, users, adjMat);
  signal(SIGUSR1, log_out);
  // Hasta aqui va la configuracion inicial de gestor
  puts("El gestor se ha configurado correctamente");

  // Creacion de pipeNom
  crear_pipe(pipe);
  // crear_pipe(pipe_registro);

  while(1) {
    printf("Esperando nuevos usuarios, enviando mi pid: %d al pipe\n", pid);
    reg(pipe, pid, users);
  }

  printf("Filename : \"%s\"\n", filename);
  return 0;
}
