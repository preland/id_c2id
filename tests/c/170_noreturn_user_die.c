#include <stdio.h>
#include <stdlib.h>

_Noreturn void die(int code) {
  printf("dying %d\n", code);
  exit(code);
}

int main(void) {
  printf("go\n");
  die(11);
  printf("after die\n");
  return 0;
}
