/* OOM RUNAWAY: GNU statement expression holding a static local struct; c2id grew past 55 GB until killed (use ulimit -v) */
#include <stdio.h>
#define S(lit) ({ static const struct { unsigned long n; char s[sizeof(lit)]; } l_ = { sizeof(lit) - 1, lit }; (char*)l_.s; })
int main(void){
  char *p = S("hello");
  int x = ({ int t = 3; t * 2; });
  printf("%s %d\n", p, x);
  return 0;}
