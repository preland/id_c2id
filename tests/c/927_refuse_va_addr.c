#include <stdio.h>
#include <stdarg.h>
static int sum(int n, ...) { va_list ap; int s = 0; va_start(ap, n); while (n-- > 0) s += va_arg(ap, int); va_end(ap); return s; }
int main(void) {
  int (*fp)(int, ...) = sum;
  printf("%d\n", fp(2, 3, 4));
  return 0;
}
