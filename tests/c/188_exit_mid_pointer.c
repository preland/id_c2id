#include <stdio.h>
#include <stdlib.h>

static int die(int code) {
  printf("dying\n");
  exit(code);
  return 0;
}

static int fine(int x) { return x + 1; }

int main(void) {
  int (*table[2])(int);
  table[0] = fine;
  table[1] = die;
  printf("%d\n", table[0](1));
  printf("%d %d\n", table[0](2), table[1](6));
  printf("unreachable\n");
  return 0;
}
