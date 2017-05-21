#include <stdlib.h>
#include <stdbool.h>
#include "util.h"
#include "tweet.h"

bool asincrono;
bool* connected;
bool** adj_mat;
int users, pointer;
pid_t* pid_users;
const char* PIPE_LECTURA = "cliente_gestor\0";
const char* PIPE_ESCRITURA = "gestor_cliente\0";
char* first_pipe;
int arrival_time[MAX_USERS];
tweet tweets[MAX_TWEETS];

void crear_pipe(const char*);
void load_file(const char*, int, bool**);
bool** reservar_matriz_booleanos(int, int);
bool* reservar_arreglo_booleanos(int);
pid_t* reservar_arreglo_pid(int);
bool valid(int);
bool conectar_cliente(int, pid_t);
void send_connection_confirmation(bool);
void send_follow_confirmation(bool);
void send_unfollow_confirmation(bool);
bool follow_action(int, int);
bool unfollow_action(int, int );
void unfollow(int);
void follow(int);
void send_tweet(pid_t, tweet*);
void tweet_handler(int);
void receive_id();
void send_pid(char*);
void end();
void desconectar_cliente(int);
void signal_handler();
