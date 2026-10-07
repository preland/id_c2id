#include <stdio.h>
int main(void) {
  char name[8] = "abcdef";
  int n = 3, v = 5;
  printf("%.*s|%+d|%#x\n", n, name, v, v);
  return 0;
}
