#include <stdio.h>
struct B {
  unsigned __int128 a : 70;
  unsigned __int128 b : 3;
  __int128 c : 40;
  int d;
};
struct N {
  __int128 s : 5;
  unsigned __int128 u : 64;
};
int main(void) {
  struct B x;
  x.a = 0;
  x.a = ((unsigned __int128)1 << 69) + 12345;
  x.b = 9;
  x.c = -3;
  x.d = 77;
  printf("%d %d %d\n", (int)(x.a >> 64), (int)(x.a & 0xffffffff), (int)x.b);
  printf("%lld %d %d\n", (long long)x.c, x.d, (int)sizeof(struct B));
  struct N n;
  n.s = 20;
  n.u = 0xfedcba9876543210ULL;
  printf("%d %llx %d\n", (int)n.s, (unsigned long long)n.u, (int)sizeof(struct N));
  return 0;
}
