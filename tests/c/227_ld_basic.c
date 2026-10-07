#include <stdio.h>

int main(void) {
  long double d = 1.5;
  d = d * 2;
  printf("%d\n", (int)d);
  long double x = 1.5L;
  long double y = x * 2 + 0.25L;
  printf("%d\n", (int)y);
  printf("%Lf %Lg\n", y, x / 3);
  printf("%.20Lf\n", (long double)1 / 3);
  printf("%d %d\n", (int)sizeof(long double), (int)sizeof(x + 1.0));
  return 0;
}
