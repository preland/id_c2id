#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

typedef long double ld;
struct P { ld a; ld b; };
struct Q { char tag; struct P p; double d; };
typedef ld (*binfn)(ld, ld);

static ld add(ld x, ld y) { return x + y; }
static ld mul(ld x, ld y) { return x * y; }
static ld idret(int n) { return n; }
static struct P mkp(ld a, ld b) { struct P p; p.a = a; p.b = b; return p; }
static ld psum(struct P p) { return p.a + p.b; }
static ld apply(binfn f, ld x, ld y) { return f(x, y); }
static ld counter(void) { static ld n = 0.5L; n += 1; return n; }
static int sgn(ld v) { return v > 0 ? 1 : (v < 0 ? -1 : 0); }
static void fmt3(ld a, ld b, ld c) { printf("%.6Lf %.6Lf %.6Lf\n", a, b, c); }
static double dsum(int n, ...) { va_list ap; double s = 0; va_start(ap, n); for (int i = 0; i < n; i++) s += va_arg(ap, double); va_end(ap); return s; }
static ld lsum(int n, ...) { va_list ap; ld s = 0; va_start(ap, n); for (int i = 0; i < n; i++) s += va_arg(ap, ld); va_end(ap); return s; }

int main(void) {
  ld x = 10, y = 3;
  ld z = x;
  z -= y;
  z *= 2;
  z /= 4;
  x += 1;
  x = y = z;
  printf("%.6Lf %.6Lf %.6Lf\n", x, y, z);
  int n = 7;
  ld w = n;
  w += n;
  w = w * n - n / 2;
  printf("%.6Lf\n", w);
  w = 1;
  w += 0.5;
  w -= 1.25f;
  printf("%.6Lf\n", w);
  ld *p = &x;
  ld arr[5];
  for (int i = 0; i < 5; i++) arr[i] = i * 0.25L;
  p = arr;
  p += 2;
  printf("%.6Lf %.6Lf %d\n", *p, p[1], (int)(p - arr));
  printf("%.6Lf\n", apply(add, 1.5L, 2.5L));
  printf("%.6Lf\n", apply(mul, 1.5L, 2.5L));
  printf("%.6Lf\n", idret(42) / 8);
  struct P q = mkp(1.5L, 2.5L);
  printf("%.6Lf %.6Lf\n", psum(q), psum(mkp(0.5L, 0.25L)));
  struct Q qq;
  memset(&qq, 0, sizeof qq);
  qq.tag = 'a';
  qq.p = q;
  qq.d = 1.75;
  printf("%.6Lf %.6f %d %d\n", qq.p.b, qq.d, (int)sizeof(qq), (int)((char *)&qq.p.b - (char *)&qq));
  counter(); counter();
  printf("%.6Lf\n", counter());
  printf("%d %d %d\n", sgn(-2.5L), sgn(0.0L), sgn(1e-4000L));
  fmt3(1, 2.5, 3.25f);
  printf("%.3f\n", dsum(3, 1.5, 2.5, 3.5));
  printf("%.3Lf\n", lsum(3, 1.5L, 2.5L, 3.5L));
  printf("%.3Lf\n", lsum(2, (ld)1, (ld)4));
  _Bool bz = 0.0L, bt = 1e-5000L, bn = 0.5L;
  printf("%d %d %d\n", bz, bt, bn);
  unsigned long big = 18446744073709551615UL;
  ld lb = big;
  printf("%.1Lf %lu\n", lb, (unsigned long)lb);
  long long mn = -9223372036854775807LL - 1;
  ld lm = mn;
  printf("%.1Lf %lld\n", lm, (long long)lm);
  ld mixed = 1 ? 2 : 3.5L;
  ld mixed2 = (n > 5) ? n : 0.5L;
  printf("%.2Lf %.2Lf\n", mixed, mixed2);
  ld c1 = (ld)1 / 3 + (ld)2 / 3;
  printf("%.20Lf\n", c1);
  printf("%Lf %Lf\n", (ld)(float)0.1, (ld)0.1);
  printf("%d\n", (int)(-2.9L));
  ld pi = 3.14159265358979323846264338327950288L;
  printf("%.30Lf\n", pi);
  printf("%Lg %Lg %Lg\n", pi * pi, pi / 1e10L, pi * 1e30L);
  printf("%10.3Lf|%-10.3Lf|%+.3Le|%08.2Lf\n", pi, pi, pi, -pi);
  printf("%Lg %Lg\n", 100000.0L, 1234567.0L);
  return 0;
}
