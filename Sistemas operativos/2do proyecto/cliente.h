#include <stdlib.h>
#include "util.h"

#define END_CHAR '$'
char* pipe_escritura;
char* pipe_lectura;

void scan_line(char[], char);
void crear_pipe(const char*);
void print_menu();
void print_tweet_menu();
void recover_tweets_option();
bool valid(char*);
bool receive_connection_confirmation();
bool receive_follow_confirmation();
bool receive_unfollow_confirmation();
int get_int();
void send_tweet_option1(int, pid_t);
void tweet_option(int, pid_t);
void follow_option(int, pid_t);
void unfollow_option(int, pid_t);
bool send_id(int);
bool receive_pid(int, pid_t*, char*);
void disconnection_option(int, pid_t);
void receive_tweets();
