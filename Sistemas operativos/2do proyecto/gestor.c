#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include "gestor.h"

/*
   TODO: Recuperar tweets
*/
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
void load_file(const char* filename, int N, bool** g) {
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
      g[i][j] = (temp == 1);
    }
  fclose(fp);
}
bool** reservar_matriz_booleanos(int rows, int columns) {
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
bool valid(int id) {
  return id >= 0 && id < users;
}
bool conectar_cliente(int id, pid_t pid) {
  bool conectado = true;
  if(valid(--id)) {
    conectado = connected[id];
    connected[id] = true;
    pid_users[id] = pid;
  }
  return conectado;
}
void send_connection_confirmation(bool answer) {
  int fd;
  puts(SEPARADOR);
  puts("Enviando la respuesta al cliente");
  if((fd = open(PIPE_ESCRITURA, O_WRONLY)) == -1) {
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
void send_follow_confirmation(bool answer) {
  send_connection_confirmation(answer);
}
void send_unfollow_confirmation(bool answer) {
  return send_connection_confirmation(answer);
}
bool follow_action(int u, int v) {
  bool ok = false;
  if(valid(v)) {
    ok = !adj_mat[u][v];
    adj_mat[u][v] = true;
  }
  if(ok)
    printf("Ahora el usuario %d sigue al usuario %d\n", 1 + u, 1 + v);
  else
    printf("La solicitud realizada por el usuario %d es invalida\n", 1 + u);
  return ok;
}
bool unfollow_action(int u, int v) {
  bool ok;
  if(valid(v)) {
    ok = adj_mat[u][v];
    adj_mat[u][v] = false;
  }
  if(ok)
    printf("El usuario %d deja de seguir al usuario %d\n", 1 + u, 1 + v);
  else
    printf("La solicitud realizada por el usuario %d es invalida\n", 1 + u);
  return ok;
}
void unfollow(int fd) {
  int u, v;
  if(read(fd, &u, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(read(fd, &v, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  send_unfollow_confirmation(unfollow_action(--u, --v));
}
void follow(int fd) {
  int u, v;
  if(read(fd, &u, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(read(fd, &v, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  send_follow_confirmation(follow_action(--u, --v));
}
void send_tweet(pid_t cliente_pid, int from, tweet* t) {
  int fd;
  kill(cliente_pid, SIGUSR1);
  if((fd = open(PIPE_ESCRITURA, O_WRONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(write(fd, &from, sizeof(int)) == -1) {
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

void tweet_handler(int fd) {
  int id, i;
  tweet t;
  if(read(fd, &id, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  if(read(fd, &t, sizeof(tweet)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  printf("El usuario %d escribio un tweet de tipo %d\n", id, t.tipo);
  if(t.tipo == 1)
    printf("El usuario con id %d escribio el tweet \"%s\"\n", id, t.text);
  else if(t.tipo == 2)
    printf("El usuario con id %d compartio una imagen, alto:%d, ancho:%d\n", id, t.imagen.alto, t.imagen.ancho);
  else if(t.tipo == 3) {
    printf("El usuario con id %d escribio el tweet \"%s\"\n", id, t.text);
    printf("Tambien compartio una imagen, alto:%d, ancho:%d\n", t.imagen.alto, t.imagen.ancho);
  }
  --id;
  if(asincrono) {
    printf("Enviandolo a los seguidores de %d\n", 1 + id);
    for(i = 0; i < users; ++i) {
      if((adj_mat[i][id] || i == id) && connected[i]) {
        printf("Como por ejemplo %d con pid %d\n", 1 + i, pid_users[i]);
        send_tweet(pid_users[i], id, &t);
      }
    }
  } else {

  }
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
  send_connection_confirmation(conectar_cliente(id, pid_cliente));
}

void send_pid(char* pipe_nom) {
  int fd, pid = getpid(), len1 = 1 + strlen(PIPE_LECTURA), len2 = 1 + strlen(PIPE_ESCRITURA);
  for(;;) {
    puts(SEPARADOR);
    puts("Enviando mi pid");
    if((fd = open(pipe_nom, O_WRONLY)) == -1) {
      perror("Error abriendo el pipe");
      exit(1);
    }
    if(write(fd, &pid, sizeof(pid_t)) == -1) {
      perror("Error escribiendo en el pipe");
      exit(1);
    }
    if(write(fd, &len1, sizeof(int)) == -1) {
      perror("");
      exit(1);
    }
    if(write(fd, PIPE_LECTURA, len1) == -1) {
      perror("Error escribiendo en el pipe");
      exit(1);
    }
    if(write(fd, &len2, sizeof(int)) == -1) {
      perror("");
      exit(1);
    }
    if(write(fd, PIPE_ESCRITURA, len2) == -1) {
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
  unlink(PIPE_ESCRITURA);
  unlink(first_pipe);
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
  puts("Leyendo del pipe, pues alguien envio una senal");
  while((fd = open(PIPE_LECTURA, O_RDONLY)) == -1) {
    perror("Error abriendo el pipe");
    exit(1);
  }
  if(read(fd, &opcion, sizeof(int)) == -1) {
    perror("Error leyendo del pipe");
    exit(1);
  }
  printf("Algun cliente escribe la opcion %d\n", opcion);
  if(opcion == DESCONEXION_ID) {
    puts("Opcion de desconexion");
    desconectar_cliente(fd);
  }
  else if(opcion == FOLLOW_ID) {
    puts("Opcion de follow");
    follow(fd);
  }
  else if(opcion == UNFOLLOW_ID) {
    puts("Opcion de unfollow");
    unfollow(fd);
  }
  else if(opcion == TWEET_ID) {
    puts("Opcion de un tweet");
    tweet_handler(fd);
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
  const char* filename = strdup(argv[2]);
  users = atoi(argv[1]);
  adj_mat = reservar_matriz_booleanos(users, users);
  connected = reservar_arreglo_booleanos(users);
  pid_users = reservar_arreglo_pid(users);
  pid_t pid = getpid();
  asincrono = (atoi(argv[3]) == ASINCRONO);
  first_pipe = strdup(argv[4]);
  signal(SIGINT, end);
  // Inicialmente no hay ningun cliente conectado
  memset(connected, false, sizeof connected);

  signal(SIGUSR1, signal_handler);
  load_file(filename, users, adj_mat);
  crear_pipe(PIPE_LECTURA);
  crear_pipe(PIPE_ESCRITURA);
  crear_pipe(argv[4]);

  // Hasta aqui va la configuracion inicial de gestor
  puts("El gestor se ha configurado correctamente");
  send_pid(first_pipe);
  return 0;
}
