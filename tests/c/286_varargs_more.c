#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>

struct P { int x; int y; };

static long long sum_ll(int n, ...) {
  va_list ap; long long s = 0;
  va_start(ap, n);
  for (int i = 0; i < n; i++) s += va_arg(ap, long long);
  va_end(ap);
  return s;
}
static unsigned long long mix_u(int n, ...) {
  va_list ap; unsigned long long s = 0;
  va_start(ap, n);
  for (int i = 0; i < n; i++) {
    s = s * 31 + va_arg(ap, unsigned int);
    s = s * 31 + va_arg(ap, unsigned long);
  }
  va_end(ap);
  return s;
}
static double many(int n, ...) {
  va_list ap; double s = 0;
  va_start(ap, n);
  for (int i = 0; i < n; i++) { int k = va_arg(ap, int); double d = va_arg(ap, double); s += k * d; }
  va_end(ap);
  return s;
}
static int walk_points(int n, ...) {
  va_list ap; int s = 0;
  va_start(ap, n);
  for (int i = 0; i < n; i++) { struct P *p = va_arg(ap, struct P *); s += p->x * 10 + p->y; }
  va_end(ap);
  return s;
}
static int fmt_into(char *buf, size_t cap, const char *fmt, ...) {
  va_list ap; int r;
  va_start(ap, fmt);
  r = vsnprintf(buf, cap, fmt, ap);
  va_end(ap);
  return r;
}
static int forward(int n, ...);
static int forward_v(int n, va_list ap) { int s = 0; for (int i = 0; i < n; i++) s = s * 3 + va_arg(ap, int); return s; }
static int forward(int n, ...) { va_list ap; va_start(ap, n); int r = forward_v(n, ap); va_end(ap); return r; }
static int count_strs(const char *first, ...) {
  va_list ap; int n = 0; const char *s = first;
  va_start(ap, first);
  while (s) { n += (int)strlen(s); s = va_arg(ap, const char *); }
  va_end(ap);
  return n;
}
static int neg(int n, ...) { va_list ap; va_start(ap, n); int a = va_arg(ap, int); int b = va_arg(ap, int); va_end(ap); return a - b + n; }
int main(void) {
  struct P a = {1, 2}, b = {3, 4};
  char buf[64];
  printf("%lld\n", sum_ll(4, 1LL << 40, -5LL, 7LL, 0x7fffffffffffffffLL));
  printf("%llu\n", mix_u(2, 4000000000u, 5000000000ul, 7u, 1ul << 63));
  printf("%.3f\n", many(9, 1, 0.5, 2, 1.5, 3, 2.5, 4, 3.5, 5, 4.5, 6, 5.5, 7, 6.5, 8, 7.5, 9, 8.5));
  printf("%d\n", walk_points(2, &a, &b));
  int r = fmt_into(buf, sizeof buf, "%d-%s-%lu-%c", -42, "abc", 123456789012UL, 'z');
  printf("%d %s\n", r, buf);
  r = fmt_into(buf, 5, "%s", "truncated");
  printf("%d %s\n", r, buf);
  printf("%d %d\n", forward(4, 1, 2, 3, 4), forward(3, 9, 8, 7));
  printf("%d\n", count_strs("ab", "cde", "f", (char *)0));
  printf("%d\n", neg(10, 3, 8));
  return 0;
}
