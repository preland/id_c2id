#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <float.h>

struct S { char c; long double x; int n; };
union U { long double x; unsigned char b[16]; };
static long double g1 = 2.5L;
static long double garr[3] = {1.0L, 0.5L, 0.25L};
long double g2;

static long double twice(long double v) { return v + v; }
static long double mix(int a, long double b, double c) { return a * b + c; }
static double sumv(int n, ...) {
  va_list ap;
  double s = 0;
  va_start(ap, n);
  while (n-- > 0) s += (double)va_arg(ap, long double);
  va_end(ap);
  return s;
}
static void show(const char *tag, long double v) { printf("%s=%.10Lf %Le %Lg\n", tag, (v < 1e30L && v > -1e30L) ? v : 0.0L, v, v); }
static void bits(const char *tag, long double v) {
  union U u;
  memset(&u, 0, sizeof u);
  u.x = v;
  printf("%s:", tag);
  for (int i = 0; i < 10; i++) printf("%02x", u.b[i]);
  printf("\n");
}

int main(void) {
  long double a = 3.0L, b = 7;
  long double c = a / b;
  show("div", c);
  bits("div", c);
  show("g1", g1);
  show("garr", garr[0] + garr[1] + garr[2]);
  g2 = twice(a) - b;
  show("g2", g2);
  show("mix", mix(2, a, 0.5));
  printf("sizeof %d %d %d\n", (int)sizeof(long double), (int)sizeof(struct S), (int)sizeof(union U));
  struct S s;
  s.c = 'x'; s.x = 1.0L / 3; s.n = 5;
  struct S t = s;
  show("sx", t.x);
  long double *p = &s.x;
  *p *= 3;
  show("sx3", s.x);
  bits("sx3", s.x);
  int i = 5;
  long double d = i;
  d += 0.1L;
  d++;
  ++d;
  d--;
  show("d", d);
  show("neg", -d);
  printf("cmp %d %d %d %d %d %d\n", a < b, a <= b, a > b, a >= b, a == b, a != b);
  printf("cmp2 %d %d\n", a < 3, 3.0 == a);
  printf("not %d %d %d\n", !a, !0.0L, a && 0);
  if (a) printf("truthy\n");
  long double z = 0.0L;
  if (!z) printf("zero\n");
  printf("tern %Lf\n", i > 3 ? a : b);
  printf("cast %d %lld %u %f %f\n", (int)b, (long long)(d * 1000), (unsigned)7.9L, (double)c, (double)(float)c);
  float f = (float)c;
  double dd = c;
  long double back = f;
  printf("float %.9f %.17g %.9Lf\n", f, dd, back);
  show("vsum", sumv(3, a, b, c));
  printf("%f\n", sumv(2, 1.5L, 2.25L));
  show("big", 1e300L * 1e300L * 1e300L);
  show("tiny", 1e-300L * 1e-300L * 1e-300L);
  show("lit", 1.18973149535723176502e+4932L);
  show("lmin", LDBL_MIN);
  show("leps", LDBL_EPSILON);
  show("inf", LDBL_MAX * 2);
  long double inf = LDBL_MAX * 2;
  long double nn = inf - inf;
  printf("%d %d\n", nn != nn, inf > LDBL_MAX);
  char buf[64];
  long double r = strtold("  -123.456e2xyz", NULL);
  snprintf(buf, sizeof buf, "%.3Lf", r);
  printf("%s\n", buf);
  char *end;
  r = strtold("1.5e-3 rest", &end);
  printf("%s|%Lg\n", end, r);
  long double arr[4];
  for (int k = 0; k < 4; k++) arr[k] = k * 1.5L;
  long double acc = 0;
  for (int k = 0; k < 4; k++) acc += arr[k];
  show("acc", acc);
  return 0;
}
