#include <stdio.h>
#include <stdarg.h>
struct P { int a; long b; };
static struct P mk(int n, ...) { va_list ap; struct P p; va_start(ap, n); p.a = va_arg(ap, int) + n; p.b = va_arg(ap, long); va_end(ap); return p; }
static int take(struct P q, int n, ...) { va_list ap; va_start(ap, n); int s = q.a + (int)q.b; for (int i = 0; i < n; i++) s += va_arg(ap, int); va_end(ap); return s; }
static const char *name(int n, ...) { va_list ap; va_start(ap, n); const char *s = va_arg(ap, const char *); va_end(ap); return n ? s : "none"; }
int main(void) {
  struct P p = mk(5, 10, 77L);
  printf("%d %ld %d %s %s\n", p.a, p.b, take(p, 2, 100, 1000), name(1, "hi"), name(0, "x"));
  struct P *pp = &p;
  printf("%d\n", take(*pp, 0));
  return 0;
}
