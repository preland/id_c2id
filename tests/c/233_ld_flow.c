#include <stdio.h>

typedef long double ld;

static int calls;
static ld probe(ld v) {
  calls++;
  return v;
}

int main(void) {
  int n = 0;
  for (ld x = 0; x < 1; x += 0.25L) n++;
  printf("%d\n", n);
  ld y = 4;
  while (y > 0.1L) y /= 2;
  printf("%.6Lf\n", y);
  ld z = 3;
  do {
    z -= 1;
  } while (z);
  printf("%.1Lf\n", z);
  ld nz = -0.0L;
  printf("%d %d %d\n", nz ? 1 : 0, !nz, nz == 0);
  ld tiny = 1e-4900L;
  printf("%d %d\n", tiny ? 1 : 0, (int)(tiny * 1e4900L));
  calls = 0;
  int a = probe(0) && probe(1);
  int b = probe(1) || probe(0);
  int c = probe(2) && probe(3);
  printf("%d %d %d %d\n", a, b, c, calls);
  ld p = +y;
  ld q = -(-y);
  printf("%d %d\n", p == y, q == y);
  ld f = 1;
  int steps = 0;
  while (f < 1e30L) {
    f *= 7;
    steps++;
  }
  printf("%d %.4Le\n", steps, f);
  ld h = 0.1L + 0.2L;
  printf("%d %d %d\n", h == 0.3L, h > 0.3L, h < 0.3L);
  printf("%d\n", (0.1L + 0.2L) == (ld)(0.1 + 0.2));
  float ff = 16777217.0L;
  double dd = 9007199254740993.0L;
  printf("%.1f %.1f\n", ff, dd);
  unsigned char uc = 200.7L;
  signed char sc = -100.9L;
  short sh = -30000.5L;
  printf("%d %d %d\n", uc, sc, sh);
  return 0;
}
