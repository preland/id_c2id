// A struct or union passed by value: the callee owns a copy.
#include <stdio.h>
struct P { int x, y; };
struct Big { int n; int a[20]; };
struct In { char c; struct P p; };
struct M { double d; char tag; long l; };
union U { int i; char b[4]; };
struct C { char c; };

int shift(struct P p) { p.x = 100; return p.x + p.y; }
int sum2(struct P a, int k, struct P b) { a.x += k; b.y -= k; return a.x + a.y + b.x + b.y; }
int total(struct Big b) { int i, s = b.n; for (i = 0; i < 20; i++) s += b.a[i]; b.a[0] = -1; return s; }
int deep(struct In v) { return v.c + v.p.x * 10 + v.p.y * 100; }
double mix(int k, struct M m, double e) { m.l += k; return m.d + m.l + m.tag + e; }
int uni(union U u) { u.i += 1; return u.b[0] + u.b[1]; }
int one(struct C c, struct C d) { return c.c + d.c; }
int viaptr(struct P p) { struct P *q = &p; q->x = 7; return p.x + q->y; }
int pass_on(struct P p) { return shift(p) + p.x; }
int rec(struct P p, int n) { if (n == 0) return p.x + p.y; p.x += 1; return rec(p, n - 1) + p.x; }
int apply(int (*f)(struct P), struct P p) { return f(p); }

int main(void) {
  struct P p = {1, 2}, q = {5, 6};
  struct P arr[3] = {{1, 1}, {2, 2}, {3, 3}};
  struct P *pp = &q;
  struct Big big;
  struct In in = {3, {4, 5}};
  struct M m = {1.5, 2, 30};
  union U u;
  struct C c1 = {5}, c2 = {6};
  int i;
  int (*fp)(struct P) = shift;
  printf("%d %d\n", shift(p), p.x);
  printf("%d\n", sum2(p, 10, q));
  printf("%d %d\n", p.x + p.y, q.x + q.y);
  printf("%d %d\n", shift(*pp), pp->x);
  printf("%d %d\n", shift(arr[2]), arr[2].x);
  for (i = 0; i < 20; i++) big.a[i] = i;
  big.n = 1000;
  printf("%d %d\n", total(big), big.a[0]);
  printf("%d\n", deep(in));
  printf("%d\n", deep(i > 0 ? in : in));
  printf("%.2f %ld\n", mix(4, m, 0.25), m.l);
  u.i = 0x0102;
  printf("%d %d\n", uni(u), u.i);
  printf("%d\n", one(c1, c2));
  printf("%d %d\n", viaptr(p), p.x);
  printf("%d %d\n", pass_on(p), p.x);
  printf("%d\n", rec(p, 5));
  printf("%d %d\n", fp(q), q.x);
  printf("%d\n", apply(shift, arr[1]));
  printf("%d\n", apply(fp, in.p));
  printf("%d\n", shift(in.p));
  printf("%d\n", sum2(arr[0], 1, arr[1]) + sum2(arr[2], 2, arr[2]));
  return 0;
}
