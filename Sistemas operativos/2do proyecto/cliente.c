#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "cliente.h"
#include "tweet.h"

void scan_line(char st[], char delim) {
  int i = 0;
  char c;
  getchar();
  while((c = getchar()) != delim)
    st[i++] = c;
  st[i] = '\0';
}
void crear_pipe(const char* pipename) {
  unlink(pipename);
  bool creado = false;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  while(!creado)
    if(mkfifo(pipename, fifo_mode) == -1) {
      perror("No se puede crear el pipe");
      printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
      sleep(SLEEP_TIME);
    } else  creado = true;
}
void print_menu() {
  puts("***MENU***");
  puts("1.Follow");
  puts("2.Unfollow");
  puts("3.Tweet");
  puts("4.Recuperar tweets");
  puts("5.Desconexion");
}
void print_tweet_menu() {
  printf("1.Mensaje de texto plano(Max longitud:%d)\n", MAX_TWEET_LENGTH - 1);
  puts("2.Imagen pequenia");
  puts("3.Mensaje de texto plano + imagen pequenia");
}
void recover_tweets_option(int id, pid_t gestor_pid) {
  int fd;
  kill(gestor_pid, SIGUSR1);
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &RECOVER_ID, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, &id, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
}
bool valid(char* arr) {
  int len = strlen(arr), i;
  for(i = 0; i < len; ++i)
    if(arr[i] < '0' || arr[i] > '9')  return false;
  return true;
}
bool receive_connection_confirmation() {
  int fd;
  bool answer;
  puts(SEPARADOR);
  printf("Abriendo el pipe(\"%s\") para recibir la confirmacion\n", pipe_lectura);
  while((fd = open(pipe_lectura, O_RDONLY)) == -1) {
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
bool receive_follow_confirmation() {
  return receive_connection_confirmation();
}
bool receive_unfollow_confirmation() {
  return receive_connection_confirmation();
}
int get_int() {
  char* buff = malloc(MAX_INPUT_CHAR * sizeof(char));
  int ans;
  if(!buff) {
    perror("Error reservando memoria para la cadena de entrada");
    exit(1);
  }
  scanf("%s", buff);
  if(valid(buff)) {
    sscanf(buff, "%d", &ans);
    return ans;
  }
  puts("Numero invalido, ingrese unicamente enteros positivos");
  return get_int();
}
void get_info_option1(tweet* t) {
  printf("Tweet(Ultimo caracter:'%c'):", END_CHAR);
  scan_line((*t).text, END_CHAR);
  (*t).tipo = 1;
  printf("El tweet es \"%s\"\n", (*t).text);
}
void get_info_option2(tweet* t) {
  bool ok = false;
  char ruta[RUTA];
  while(!ok) {
    printf("Ruta de la imagen:");
    scan_line(ruta, '\n');
    if(abrir_imagen(&((*t).imagen), ruta))
      ok = true;
  }
  (*t).tipo = 2;
}
void get_info_option3(tweet* t) {
  get_info_option1(t);
  get_info_option2(t);
  (*t).tipo = 3;
}
void send_tweet(pid_t gestor_pid, tweet* t) {
  int fd;
  kill(gestor_pid, SIGUSR1);
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &TWEET_ID, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, t, sizeof(tweet)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
}
void tweet_option(int id, pid_t gestor_pid) {
  tweet t;
  int opcion;
  bool valid = false;
  t.from = id;
  while(!valid) {
    print_tweet_menu();
    opcion = get_int();
    valid = true;
    switch(opcion) {
      case 1:
        get_info_option1(&t);
        send_tweet(gestor_pid, &t);
        break;
      case 2:
        get_info_option2(&t);
        send_tweet(gestor_pid, &t);
        break;
      case 3:
        get_info_option3(&t);
        send_tweet(gestor_pid, &t);
        break;
      default:
        puts("Opcion desconocida, intente de nuevo");
        valid = false;
    }
  }
}
void follow_option(int id, pid_t gestor_pid) {
  int follow, fd;
  printf("Identificador del usuario al que desea seguir:");
  follow = get_int();
  kill(gestor_pid, SIGUSR1);
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &FOLLOW_ID, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, &id, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, &follow, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    puts("Error cerrando el pipe");
    exit(1);
  }
  if(receive_follow_confirmation())
    printf("Ahora se sigue al usuario con id %d\n", follow);
  else  puts("Ya se sigue al usuario o el id es invalido\n");
}
void unfollow_option(int id, pid_t gestor_pid) {
  int fd, unfollow;
  printf("Identificador del usuario al que desea dejar de seguir:");
  unfollow = get_int();
  kill(gestor_pid, SIGUSR1);
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &UNFOLLOW_ID, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, &id, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(write(fd, &unfollow, sizeof(int)) == -1) {
    perror("Error escribiendo en el pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  if(receive_unfollow_confirmation())
    printf("Ya no se sigue al usuario %d\n", unfollow);
  else
    puts("No se seguia al usuario o el id es invalido");
}
bool send_id(int id) {
  int fd, mipid = getpid();
  puts(SEPARADOR);
  puts("Abriendo el pipe para enviar mi id, pid");
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
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
  return receive_connection_confirmation();
}
bool receive_pid(int id, pid_t* gestor_pid, char* pipe_nom) {
  int fd, l1, l2;
  puts(SEPARADOR);
  puts("Obteniendo el pid del gestor");
  while((fd = open(pipe_nom, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
  }
  if(read(fd, gestor_pid, sizeof(pid_t)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(read(fd, &l1, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  pipe_escritura = malloc(l1 * sizeof(char));
  if(!pipe_escritura) {
    perror("Error reservando memoria para la cadena del pipe:");
    exit(1);
  }
  if(read(fd, pipe_escritura, l1) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(read(fd, &l2, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  pipe_lectura = malloc(l2 * sizeof(char));
  if(!pipe_lectura) {
    perror("Error reservando memoria para la cadena del pipe:");
    exit(1);
  }
  if(read(fd, pipe_lectura, l2) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  printf("Recibido, el pid del gestor: %d\n", *gestor_pid);
  printf("El pipe para escribir al gestor es \"%s\"\n", pipe_escritura);
  printf("El pipe para leer del gestor es \"%s\"\n", pipe_lectura);
  return send_id(id);
}
void disconnection_option(int id, pid_t gestor_pid) {
  int fd;
  kill(gestor_pid, SIGUSR1);
  if((fd = open(pipe_escritura, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &DESCONEXION_ID, sizeof(int)) == -1) {
    perror("Error escribiendo al pipe");
    exit(1);
  }
  if(write(fd, &id, sizeof(int)) == -1) {
    perror("Error escribiendo al pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  exit(1);
}
void print_tweet(tweet t) {
  puts(SEPARADOR);
  printf("El usuario %d escribio un tweet de tipo %d\n", t.from, t.tipo);
  if(t.tipo == 1)
    printf("El usuario con id %d escribio el tweet \"%s\"\n", t.from, t.text);
  else if(t.tipo == 2)
    printf("El usuario con id %d compartio una imagen, alto:%d, ancho:%d\n", t.from, t.imagen.alto, t.imagen.ancho);
  else if(t.tipo == 3) {
    printf("El usuario con id %d escribio el tweet \"%s\"\n", t.from, t.text);
    printf("Tambien compartio una imagen, alto:%d, ancho:%d\n", t.imagen.alto, t.imagen.ancho);
  }
}
void receive_tweets() {
  int fd;
  tweet t;
  while((fd = open(pipe_lectura, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
  }
  if(read(fd, &t, sizeof(tweet)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(close(fd) == -1) {
    perror("Error cerrando el pipe");
    exit(1);
  }
  print_tweet(t);
}
void receive_recovered_tweets() {
  int fd, cuantos, i;
  tweet t;
  while((fd = open(pipe_lectura, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    printf("Intentando de nuevo en %d segundos\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
  }
  if(read(fd, &cuantos, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  printf("Se recuperaron %d tweets\n", cuantos);
  for(i = 0; i < cuantos; ++i) {
    if(read(fd, &t, sizeof(tweet)) == -1) {
      perror("Error leyendo del pipe");
      exit(1);
    }
    print_tweet(t);
  }
}

int main(int argc, char* argv[]) {
  if(argc != 3) {
    printf("Error, usage: %s ID pipeNom\n", argv[0]);
    exit(1);
  }
  int id = atoi(argv[1]), opcion;
  pid_t* gestor_pid = malloc(sizeof(pid_t));
  if(!gestor_pid) {
    perror("Error reservando memoria al pid del gestor");
    exit(1);
  }
  signal(SIGUSR1, receive_tweets);
  signal(SIGUSR2, receive_recovered_tweets);
  printf("Iniciando el cliente, mi pid es: %d\n", getpid());
  if(receive_pid(id, gestor_pid, argv[2])) {
    printf("El usuario con id %d ya se encuentra registrado o es invalido\n", id);
    exit(1);
  }
  puts("Cliente iniciado");
  for(;;) {
    print_menu();
    opcion = get_int();
    switch(opcion) {
      case 1:
        follow_option(id, *gestor_pid);
        break;
      case 2:
        unfollow_option(id, *gestor_pid);
        break;
      case 3:
        tweet_option(id, *gestor_pid);
        break;
      case 4:
        recover_tweets_option(id, *gestor_pid);
        break;
      case 5:
        disconnection_option(id, *gestor_pid);
        break;
      default:
        puts("Opcion desconocida, intente de nuevo");
    }
  }
  return 0;
}
