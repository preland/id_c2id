#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static int sum(int n, ...) { va_list ap; int s = 0; va_start(ap, n); for (int i = 0; i < n; i++) s += va_arg(ap, int); va_end(ap); return s; }
static double avg(int n, ...) { va_list ap; double t = 0; va_start(ap, n); for (int i = 0; i < n; i++) t += va_arg(ap, double); va_end(ap); return n ? t / n : 0; }
static long mix(const char *fmt, ...) {
  va_list ap; long acc = 0;
  va_start(ap, fmt);
  for (const char *p = fmt; *p; p++) {
    if (*p == 'i') acc += va_arg(ap, int);
    else if (*p == 'l') acc += va_arg(ap, long);
    else if (*p == 's') acc += (long)strlen(va_arg(ap, char *));
    else if (*p == 'd') acc += (long)(va_arg(ap, double) * 10);
    else if (*p == 'p') acc += (va_arg(ap, void *) != 0);
  }
  va_end(ap);
  return acc;
}
static int take_str(va_list ap) { char *s = va_arg(ap, char *); int n = va_arg(ap, int); return (int)strlen(s) + n; }
static int via(const char *tag, ...) { va_list ap; va_start(ap, tag); int r = take_str(ap); va_end(ap); return r + (int)strlen(tag); }
static int copy2(int n, ...) { va_list a, b; va_start(a, n); va_copy(b, a); int x = 0; for (int i = 0; i < n; i++) x += va_arg(a, int); for (int i = 0; i < n; i++) x += 100 * va_arg(b, int); va_end(a); va_end(b); return x; }
static int rec(int n, ...) { va_list ap; va_start(ap, n); int v = n > 0 ? va_arg(ap, int) : 0; va_end(ap); return n > 0 ? v + rec(n - 1, v + 1) : 0; }
static void line(const char *fmt, ...) { va_list ap; va_start(ap, fmt); int a = va_arg(ap, int); const char *b = va_arg(ap, const char *); va_end(ap); printf(fmt, a, b); }
int main(void) {
  float f = 1.5f; char c = 'A'; short sh = -7; unsigned char uc = 250;
  printf("%d %d %d\n", sum(0), sum(3, 1, 2, 3), sum(8, 1, 2, 3, 4, 5, 6, 7, 8));
  printf("%.2f %.2f\n", avg(3, 1.0, 2.0, 4.5), avg(2, f, 2.5));
  printf("%ld\n", mix("ilsdp", 5, 1L << 40, "hello", 2.5, (void *)&c));
  printf("%ld\n", mix("iii", c, sh, uc));
  printf("%d %d %d\n", via("ab", "cde", 4), copy2(3, 1, 2, 3), rec(4, 0));
  line("%d-%s\n", 7, "x");
  return 0;
}
