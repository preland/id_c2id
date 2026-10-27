#include <stdio.h>
#include <stdlib.h>

int main(void) {
  printf("before\n");
  fflush(stdout);
  abort();
  printf("after abort\n");
  return 0;
}
