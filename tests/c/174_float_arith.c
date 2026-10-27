#include <stdio.h>

static double sq(double x) { return x * x; }
static float halve(float x) { return x / 2; }
static double mix(int a, double b, float c) { return a + b * c; }

int main(void) {
  double a = 1.5, b = 2.25, c;
  float f = 0.1f, g;
  double z = 0.0, nz = -0.0, nan = 0.0 / 0.0;
  int i;
  unsigned u = 4000000000u;
  double du = u;
  c = a * b + 0.125;
  printf("%d\n", (int)(c * 1000));
  g = f * 3.0f;
  printf("%d\n", (int)(g * 1e7));
  printf("%d\n", (int)(f * 1e9));
  printf("%d %d\n", a < b, a > b);
  printf("%d %d %d\n", a == 1.5, a != 1.5, a <= 1.5);
  printf("%d\n", (int)sq(3));
  printf("%d\n", (int)(halve(5) * 10));
  printf("%d\n", (int)(mix(2, 1.5, 2) * 100));
  for (i = 0; i < 10; i++) {
    c += 0.1;
  }
  printf("%d\n", (int)(c * 1e6));
  printf("%d %d\n", (int)-2.9, (int)2.9);
  printf("%d\n", (long)1e10 > 0);
  printf("%d %d\n", !z, !a);
  printf("%d\n", nz == 0.0);
  if (nz) {
    printf("negative zero is true\n");
  } else {
    printf("negative zero is false\n");
  }
  printf("%d %d %d %d\n", nan == nan, nan != nan, nan < 1.0, nan >= 1.0);
  if (nan) {
    printf("nan is true\n");
  }
  printf("%d\n", (int)(1.0 / 3 * 3 * 1000));
  printf("%d\n", (int)(du / 1000000));
  printf("%d\n", (int)(-a * 10));
  printf("%d\n", (int)(float)16777217);
  c = 7;
  c /= 2;
  c *= 4;
  c -= 0.5;
  printf("%d\n", (int)(c * 10));
  i = 3;
  i *= 1.5;
  printf("%d\n", i);
  c = 1.5;
  c++;
  ++c;
  c--;
  printf("%d\n", (int)(c * 10));
  return 0;
}
