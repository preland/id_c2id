#include <stdio.h>
#include <stdlib.h>

int main(void) {
  printf("one\n");
  fflush(stdout);
  _Exit(9);
  printf("after _Exit\n");
  return 0;
}
