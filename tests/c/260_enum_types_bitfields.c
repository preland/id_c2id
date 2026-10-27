#include <stdio.h>
enum Mode { M_A, M_B, M_C, M_D };
enum Signed { S_NEG = -2, S_A = 1, S_B = 2 };
typedef enum { T_X, T_Y, T_Z } tmode;
struct S { enum Mode m : 2; unsigned n : 6; enum Signed s : 3; tmode t : 2; };
int main(void) {
  struct S s;
  enum Mode e = M_D;
  enum Signed g = S_NEG;
  s.m = M_D;
  s.n = 1;
  s.s = S_NEG;
  s.t = T_Z;
  printf("%d %u %d %d\n", (int)s.m, s.n, (int)s.s, (int)s.t);
  printf("%d %d %d %d\n", e - 5 < 0, g - 5 < 0, (int)(e - 5), (int)(g * 3));
  printf("%d %d\n", (int)sizeof(enum Mode), s.m == M_D);
  return 0;
}
