#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "util.h"

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


void tweet_option() {

}
void recover_tweets_option() {

}
void disconnection_option(const char* pipename, int id, pid_t gestor_pid) {
  kill(gestor_pid, SIGUSR1);
  int fd;

  fd = open(pipename, O_WRONLY);
  write(fd, &id, sizeof(int));
  close(fd);

  exit(1);
}

bool connect(const char* pipename, int id) {
  int fd;
  bool registrado;

  fd = open(pipename, O_WRONLY);
  write(fd, &id, sizeof(int));
  close(fd);

  fd = open(pipename, O_RDONLY);
  read(fd, &registrado, sizeof(bool));
  close(fd);

  return registrado;
}

bool init(const char* pipename, int id, pid_t* gestor_pid) {
  int fd;
  puts("Obteniendo el pid del gestor");

  fd = open(pipename, O_RDONLY);
  read(fd, gestor_pid, sizeof(pid_t));
  close(fd);

  printf("El pid del gestor es %d\n", *gestor_pid);
  puts("Registrandome en el gestor");

  return connect(pipename, id);
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    printf("Error, usage: %s ID pipeNom\n", argv[0]);
    exit(1);
  }

  int id = atoi(argv[1]), opcion;
  const char* pipename = strdup(argv[2]);
  pid_t* gestor_pid = malloc(sizeof(pid_t));

  if(!gestor_pid) {
    perror("Error reservando memoria al pid del gestor: ");
    exit(1);
  }

  puts("Iniciando el cliente");

  if(init(pipename, id, gestor_pid)) {
    printf("El usuario con id %d ya se encuentra registrado o es invalido\n", id);
    exit(1);
  }

  puts("Cliente iniciado");

  while(1) {
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
        disconnection_option(pipename, id, *gestor_pid);
        break;
      default:
        puts("Opcion desconocida, intente de nuevo");
    }
  }
  return 0;
}
