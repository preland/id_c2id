#include <stdio.h>
#include <stdlib.h>

void fatal(const char *m) {
  printf("fatal %s\n", m);
  exit(2);
}

void check(int x) {
  if (x > 2) {
    fatal("big");
  }
}

int main(void) {
  int i;
  for (i = 0; i < 5; i++) {
    check(i);
    printf("ok %d\n", i);
  }
  printf("done\n");
  return 0;
}
