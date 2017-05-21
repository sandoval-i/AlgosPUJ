#include <stdlib.h>
#include "bmp.c"

#define MAX_TWEET_LENGTH 150

typedef struct tweet {
  int tipo; // tipo pertenece a {1|2|3}
  BMP imagen;
  char text[MAX_TWEET_LENGTH];
}tweet;
