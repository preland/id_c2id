#include <stdio.h>
enum Mode { M_A, M_B, M_C, M_D };
struct S { enum Mode m : 2; unsigned n : 6; };
int main(void) {
  struct S s;
  s.m = M_D;
  s.n = 1;
  printf("%d %u\n", (int)s.m, s.n);
  return 0;
}
