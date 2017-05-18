#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "util.h"

bool conectado;
char* pipe;

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
void print_menu() {
  puts("***MENU***");
  puts("1.Follow");
  puts("2.Unfollow");
  puts("3.Tweet");
  puts("4.Recuperar tweets");
  puts("5.Desconexion");
}
void follow_option() {
  int id;
  printf("Identificador del usuario al que desea seguir:");
  scanf("%d", &id);
}
void unfollow_option() {
  int id;
  printf("Identificador del usuario al que desea dejar de seguir:");
  scanf("%d", &id);
}
void tweet_option() {}
void recover_tweets_option() {}

bool receive_confirmation(const char* pipename) {
  int fd;
  bool answer;
  puts(separador);
  puts("Abriendo el pipe para recibir la confirmacion");
  while((fd = open(pipename, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
  }
  if(read(fd, &answer, sizeof(bool)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  printf("Confirmacion recibida: %d\n", answer);
  return answer;

}

bool send_id(const char* pipename, int id) {
  int fd, mipid = getpid();
  crear_pipe(pipename);
  puts(separador);
  puts("Abriendo el pipe para enviar mi id, pid");
  if((fd = open(pipename, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &id, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, &mipid, sizeof(pid_t)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  puts("Id, pid enviado");
  unlink(pipename);
  return receive_confirmation(pipename);
}

bool receive_pid(const char* pipename, int id, pid_t* gestor_pid) {
  int fd;
  puts(separador);
  puts("Obteniendo el pid del gestor");
  while((fd = open(pipename, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
  }
  if(read(fd, gestor_pid, sizeof(pid_t)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  printf("Recibido, el pid del gestor es %d\n", *gestor_pid);
  return send_id(pipe_registro, id);
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    printf("Error, usage: %s ID pipeNom\n", argv[0]);
    exit(1);
  }
  int id = atoi(argv[1]), opcion;
  pipe = strdup(argv[2]);
  pid_t* gestor_pid = malloc(sizeof(pid_t));
  if(!gestor_pid) {
    perror("Error reservando memoria al pid del gestor");
    exit(1);
  }
  printf("Iniciando el cliente, mi pid es: %d\n", getpid());
  if(receive_pid(pipe, id, gestor_pid)) {
    printf("El usuario con id %d ya se encuentra registrado o es invalido\n", id);
    exit(1);
  }
  puts("Cliente iniciado");
  for(;;) {
    print_menu();
    scanf("%d", &opcion);
    switch(opcion) {
      case 1:
        follow_option();
        break;
      case 2:
        unfollow_option();
        break;
      case 3:
        tweet_option();
        break;
      case 4:
        recover_tweets_option();
        break;
      case 5:
        // disconnection_option(pipe, id, *gestor_pid);
        break;
      default:
        puts("Opcion desconocida, intente de nuevo");
    }
  }
  return 0;
}
