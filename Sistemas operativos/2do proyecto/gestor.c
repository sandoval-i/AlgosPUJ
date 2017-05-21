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
char* pipe_escritura;
int users;
pid_t* pid_users;
const char* PIPE_LECTURA = "registro\0";

void crear_pipe(const char* pipename) {
  // printf("Creando un pipe con nombre \"%s\"\n", pipename);
  unlink(pipename);
  bool creado = false;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  while(!creado)
    if(mkfifo(pipename, fifo_mode) == -1) {
      perror("No se puede crear el pipe");
      printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
      sleep(SLEEP_TIME);
    } else  creado = true;
    // printf("Pipe con nombre \"%s\" creado\n", pipename);
}
void load_file(const char* filename, int N, bool** adjMat) { // Lee el archivo que contiene el grafo de relaciones iniciales
  int i,j, temp;
  FILE* fp = fopen(filename, "r");
  if(!fp) {
    perror("Error abriendo el archivo de relaciones");
    exit(1);
  }
  for(i = 0; i < N; ++i)
    for(j = 0; j < N; ++j) {
      if(fscanf(fp, "%d", &temp) == EOF) {
        perror("Error leyendo el archivo de relaciones");
        exit(1);
      }
      adjMat[i][j] = (temp == 1);
    }
  fclose(fp);
}
bool** reservar_matriz_booleanos(int rows, int columns) { // Reserva espacio para una matriz de booleanos de rows * columns
  int i;
  bool** mat = malloc(rows * sizeof(bool*));;
  if(!mat) {
    perror("Error reservando memoria para una matriz de booleanos");
    exit(1);
  }
  for(i = 0; i < rows; ++i) {
    mat[i] = malloc(columns * sizeof(bool));
    if(!mat) {
      perror("Error reservando memoria para una matriz de booleanos");
      exit(1);
    }
  }
  return mat;
}
bool* reservar_arreglo_booleanos(int rows) {
  bool* arr = malloc(rows * sizeof(bool));
  if(!arr) {
    perror("Error reservando memoria para un arreglo de booleanos");
    exit(1);
  }
  return arr;
}
pid_t* reservar_arreglo_pid(int rows) {
  pid_t* arr = malloc(rows * sizeof(pid_t));
  if(!arr) {
    perror("Error reservando memoria para un arreglode pid's");
    exit(1);
  }
  return arr;
}
bool conectar_cliente(int id, pid_t pid) {
  bool conectado = true;
  if(--id >= 0 && id < users) {
    conectado = connected[id];
    connected[id] = true;
    pid_users[id] = pid;
  }
  return conectado;
}

void send_confirmation(bool answer) {
  int fd;
  puts(SEPARADOR);
  puts("Enviando la respuesta al cliente");
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &answer, sizeof(bool)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  puts("Sent!");
}

void receive_id() {
  int id, fd;
  pid_t pid_cliente;
  puts(SEPARADOR);
  puts("Leyendo el id, pid del cliente");
  while((fd = open(PIPE_LECTURA, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
  }
  if(read(fd, &id, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(read(fd, &pid_cliente, sizeof(pid_t)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  printf("El cliente con id %d y pid %d se quiere registrar\n", id, pid_cliente);
  send_confirmation(conectar_cliente(id, pid_cliente));
}

/* Funcion que se ejecuta infinitamente y envia el pid a los clientes
a traves del pipe pipename */
void send_pid() {
  int fd, pid = getpid(), i = 0;
  for(;;) {
    puts(SEPARADOR);
    puts("Enviando mi pid");
    if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
      perror("Error abriendo el pipe");
      exit(1);
    }
    if(write(fd, &pid, sizeof(pid_t)) == -1) {
      perror("Error escribiendo en el pipe");
      exit(1);
    }
    if(write(fd, PIPE_LECTURA, sizeof(PIPE_LECTURA)) == -1) {
      perror("Error escribiendo en el pipe");
      exit(1);
    }
    if(close(fd) == -1) {
      perror("Error cerrando el pipe");
      exit(1);
    }
    puts("Sent!");
    receive_id();
  }
}

void end() {
  unlink(PIPE_LECTURA);
  unlink(pipe_escritura);
  puts("");
  exit(1);
}

void desconectar_cliente(int fd) {
  int id;
  if(read(fd, &id, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  connected[--id] = false;
  printf("El cliente con id %d fue desconectado\n", 1 + id);
}

void signal_handler() {
  int fd, opcion;
  puts(SEPARADOR);
  puts("Leyendo del pipe");
  while((fd = open(PIPE_LECTURA, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  puts("OPEN!");
  if(read(fd, &opcion, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  printf("Algun cliente escribe la opcion %d\n", opcion);
  if(opcion == DESCONEXION_ID) {
    puts("Opcion de desconexion");
    desconectar_cliente(fd);
  }
  else
    puts("Opcion desconocida");
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  if(argc != 5) {
    printf("Error, usage: %s N relaciones modo pipeNom\n", argv[0]);
    exit(1);
  }

  int modo = atoi(argv[3]);
  const char* filename = strdup(argv[2]);
  users = atoi(argv[1]);
  pipe_escritura = strdup(argv[4]);
  bool** adjMat = reservar_matriz_booleanos(users, users);
  connected = reservar_arreglo_booleanos(users);
  pid_users = reservar_arreglo_pid(users);
  pid_t pid = getpid();
  signal(SIGINT, end);
  // Inicialmente no hay ningun cliente conectado
  memset(connected, false, sizeof connected);

  signal(SIGUSR1, signal_handler);
  load_file(filename, users, adjMat);
  crear_pipe(PIPE_LECTURA);
  crear_pipe(pipe_escritura);

  // Hasta aqui va la configuracion inicial de gestor
  puts("El gestor se ha configurado correctamente");
  send_pid();
  return 0;
}
