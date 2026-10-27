#include <stdio.h>
#include <stdlib.h>

_Noreturn void die(int code) {
  printf("dying %d\n", code);
  exit(code);
}

void quit(int code) __attribute__((noreturn));

__attribute__((noreturn)) void halt(int code);

void quit(int code) {
  printf("quitting %d\n", code);
  exit(code);
}

void halt(int code) {
  printf("halting %d\n", code);
  exit(code);
}

int pick(int n) {
  if (n == 1) {
    die(11);
    printf("after die\n");
  }
  if (n == 2) {
    quit(12);
    printf("after quit\n");
  }
  halt(13);
  printf("after halt\n");
  return n;
}

int main(void) {
  printf("pick 0\n");
  pick(0);
  printf("not reached\n");
  return 0;
}
