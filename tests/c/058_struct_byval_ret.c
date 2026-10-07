// A struct or union returned by value: each call site owns its result.
#include <stdio.h>
struct P { int x, y; };
struct Big { int n; int a[20]; };
struct In { char c; struct P p; };
struct W { struct P lo, hi; };
union U { int i; char b[4]; };
struct Pair { long a, b; };

struct P make(int a, int b) { struct P p; p.x = a; p.y = b; return p; }
struct P add(struct P a, struct P b) { struct P r = a; r.x += b.x; r.y += b.y; return r; }
struct P same(struct P p) { return p; }
struct P twice(struct P p) { return add(p, p); }
struct P viacall(int a) { return make(a, a + 1); }
struct P *where(struct P *p) { return p; }
struct P deref(struct P *p) { return *p; }
struct Big big(int n) { struct Big b; int i; b.n = n; for (i = 0; i < 20; i++) b.a[i] = n * i; return b; }
struct In inner(int k) { struct In v; v.c = k; v.p = make(k, k * 2); return v; }
struct W wide(int k) { struct W w; w.lo = make(k, 0); w.hi = make(0, k); return w; }
union U uni(int i) { union U u; u.i = i; return u; }
struct Pair fib(int n) { struct Pair r; struct Pair q; if (n == 0) { r.a = 0; r.b = 1; return r; } q = fib(n - 1); r.a = q.b; r.b = q.a + q.b; return r; }
struct P pick(int c, struct P a, struct P b) { if (c) return a; return b; }
struct P counter(void) { static int n; struct P p; p.x = ++n; p.y = n * n; return p; }
void noise(void) { make(1, 2); counter(); }

int main(void) {
  struct P p = make(3, 4), q, r;
  struct P arr[3];
  struct P (*mk)(int, int) = make;
  struct P (*tbl[2])(struct P, struct P) = { add, add };
  struct Big b = big(3);
  struct W w;
  union U u = uni(258);
  int i, s = 0;
  printf("%d %d\n", p.x, p.y);
  printf("%d %d\n", make(5, 6).x, make(5, 6).y);
  printf("%d\n", make(1, 2).x + make(10, 20).y);
  q = make(7, 8);
  printf("%d %d\n", q.x, q.y);
  q = same(q);
  printf("%d %d\n", q.x, q.y);
  q = add(q, make(1, 1));
  printf("%d %d\n", q.x, q.y);
  r = add(make(1, 2), make(30, 40));
  printf("%d %d\n", r.x, r.y);
  r = twice(add(make(1, 1), twice(make(2, 2))));
  printf("%d %d\n", r.x, r.y);
  printf("%d %d\n", viacall(9).x, viacall(9).y);
  printf("%d\n", where(&q)->x + deref(&q).y);
  printf("%d %d\n", b.n, b.a[19]);
  printf("%d %d\n", big(5).a[7], big(6).a[2] + big(7).n);
  printf("%d %d %d\n", inner(4).c, inner(4).p.x, inner(5).p.y);
  w = wide(6);
  printf("%d %d %d %d\n", w.lo.x, w.lo.y, w.hi.x, w.hi.y);
  printf("%d\n", wide(2).hi.y + wide(3).lo.x);
  printf("%d %d\n", u.i, uni(0x0304).b[1]);
  printf("%ld %ld\n", fib(10).a, fib(30).b);
  printf("%d\n", pick(1, make(1, 2), make(3, 4)).y);
  printf("%d\n", pick(0, make(1, 2), make(3, 4)).y);
  for (i = 0; i < 5; i++) { arr[i % 3] = counter(); s += arr[i % 3].y; }
  printf("%d %d %d %d\n", s, arr[0].x, arr[1].x, arr[2].x);
  noise();
  printf("%d\n", counter().x);
  printf("%d %d\n", mk(2, 3).x, mk(2, 3).y);
  r = mk(8, 9);
  printf("%d %d\n", r.x, r.y);
  printf("%d\n", tbl[1](make(1, 1), mk(2, 2)).x);
  p = make(p.y, p.x);
  printf("%d %d\n", p.x, p.y);
  p = add(p, p);
  printf("%d %d\n", p.x, p.y);
  return 0;
}
