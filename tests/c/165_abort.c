#include <stdio.h>
#include <stdlib.h>

int main(void) {
  printf("before\n");
  fflush(stdout);
  abort();
  return 0;
}
