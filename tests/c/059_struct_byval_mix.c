// Structs by value in the contexts around a call: member access, conditions,
// loops, tables of function pointers, callees that exit, and larger signatures.
#include <stdio.h>
#include <stdlib.h>
struct Fwd;
struct Fwd later(int k);
typedef struct { double x, y; } V;
typedef struct { int tag; V v; } Tagged;
struct Ops { struct Fwd (*get)(int); int (*len)(V); };
struct Fwd { int a; int b; };
struct Six { int a, b, c, d, e, f; };

struct Fwd later(int k) { struct Fwd r; r.a = k; r.b = k + 100; return r; }
V vadd(V a, V b) { V r; r.x = a.x + b.x; r.y = a.y + b.y; return r; }
V vscale(V a, double k) { a.x *= k; a.y *= k; return a; }
int vlen2(V a) { return (int)(a.x * a.x + a.y * a.y); }
Tagged tag(int t, V v) { Tagged r; r.tag = t; r.v = v; return r; }
struct Six six(int a, int b, int c, int d, int e, int f) { struct Six s; s.a = a; s.b = b; s.c = c; s.d = d; s.e = e; s.f = f; return s; }
int sixsum(struct Six s, int a, int b, int c, int d, int e) { return s.a + s.b + s.c + s.d + s.e + s.f + a + b + c + d + e; }
struct Fwd bye(int k) { if (k > 0) { printf("bye %d\n", k); exit(7); } return later(0); }
struct Fwd pick(int c, struct Fwd a, struct Fwd b) { return c ? a : b; }
struct Fwd chain3(int n) { struct Fwd r = later(n); if (n > 0) r.a += chain3(n - 1).a; return r; }

int main(void) {
  V a = {1.5, 2.0}, b = {0.25, 4.0}, c;
  struct Ops ops = {later, vlen2};
  struct Fwd f, g;
  struct Fwd *heap = malloc(sizeof(struct Fwd));
  Tagged t;
  int i, s = 0;
  struct Fwd (*fp)(int) = later;
  c = vadd(a, vscale(b, 2.0));
  printf("%.2f %.2f\n", c.x, c.y);
  printf("%d %d\n", ops.len(c), ops.len(vadd(a, a)));
  printf("%d %d\n", ops.get(5).a, ops.get(5).b);
  t = tag(3, vadd(c, a));
  printf("%d %.2f %.2f\n", t.tag, t.v.x, t.v.y);
  printf("%.2f\n", tag(1, a).v.y + tag(2, b).v.x);
  f = later(1);
  g = (f = later(2));
  printf("%d %d\n", f.a, g.a);
  printf("%d\n", (f = later(9)).b);
  printf("%d\n", f.a);
  *heap = later(40);
  printf("%d %d\n", heap->a, later(heap->b).a);
  printf("%d\n", (int)sizeof(later(1)) + (int)sizeof(later(1).b));
  printf("%d\n", pick(s == 0, later(1), later(2)).a);
  printf("%d\n", pick(s != 0, later(1), later(2)).a);
  for (i = 0; later(i).a < 5; i++) s += later(i).b;
  printf("%d %d\n", i, s);
  i = 0;
  do { s += later(i).a; i++; } while (later(i).a < 4);
  printf("%d\n", s);
  while (later(s).a > 100) s -= 50;
  printf("%d\n", s);
  printf("%d\n", later(1).a ? later(2).b : later(3).b);
  printf("%d\n", later(0).a && later(5).b > 100);
  printf("%d %d\n", fp(3).a + fp(4).a, fp(fp(2).a).b);
  printf("%d\n", sixsum(six(1, 2, 3, 4, 5, 6), 10, 20, 30, 40, 50));
  printf("%d %d\n", six(1, 2, 3, 4, 5, 6).f, six(6, 5, 4, 3, 2, 1).a);
  printf("%d\n", chain3(10).a);
  printf("%d\n", bye(0).b);
  printf("%d\n", bye(2).b);
  printf("not reached\n");
  return 0;
}
