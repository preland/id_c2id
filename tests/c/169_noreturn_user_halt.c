#include <stdio.h>
#include <stdlib.h>

__attribute__((noreturn)) void halt(int code);

void halt(int code) {
  printf("halting %d\n", code);
  exit(code);
}

int main(void) {
  printf("go\n");
  halt(13);
  printf("after halt\n");
  return 0;
}
