#include <stdio.h>
#include <stdlib.h>

static int die(int code) {
  printf("dying\n");
  exit(code);
  return 0;
}

static int fine(int x) { return x + 1; }

static int arm(int c) { return c ? die(4) : 9; }

int main(int argc, char **argv) {
  int (*fp)(int) = fine;
  printf("%d\n", fine(1));
  printf("%d\n", fp(2) + fine(3));
  printf("%d\n", arm(0));
  if (argc > 5 || fine(1) == 2) {
    printf("%d\n", arm(0) + fine(0));
  }
  printf("%d %d\n", fine(1), arm(1));
  printf("unreachable\n");
  return 0;
}
