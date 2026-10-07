/* SILENT MISCOMPILE: same-named typedef local to two functions collides; prints '24 24', gcc prints '24 808' */
#include <stdio.h>
typedef struct { long a[2]; } Small;
typedef struct { long a[100]; } Big;
static size_t sz_small(void) { typedef struct { char c; Small t; } t_a; return sizeof(t_a); }
static size_t sz_big(void) { typedef struct { char c; Big t; } t_a; return sizeof(t_a); }
int main(void){
  printf("%d %d\n", (int)sz_small(), (int)sz_big());
  return 0;}
