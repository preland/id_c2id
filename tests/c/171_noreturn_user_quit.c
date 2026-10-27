#include <stdio.h>
#include <stdlib.h>

void quit(int code) __attribute__((noreturn));

void quit(int code) {
  printf("quitting %d\n", code);
  exit(code);
}

int main(void) {
  printf("go\n");
  quit(12);
  printf("after quit\n");
  return 0;
}
