#include <stdio.h>
struct B { unsigned a : 3; unsigned b : 5; };
int main(void) {
  struct B x;
  x.a = 5;
  x.b = 17;
  printf("%d %d\n", x.a, x.b);
  return 0;
}
