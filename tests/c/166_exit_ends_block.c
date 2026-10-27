#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  printf("start\n");
  if (argc > 0) {
    printf("leaving\n");
    exit(7);
    printf("after exit\n");
  }
  printf("fell through\n");
  return 0;
}
