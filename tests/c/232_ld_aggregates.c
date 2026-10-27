#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long double ld;
struct P { ld a; ld b; };
struct N { int tag; struct P p; ld arr[2]; };
union V { ld x; struct { unsigned long long m; unsigned short se; } r; };

static ld sum(const ld *v, int n) {
  ld s = 0;
  for (int i = 0; i < n; i++) s += v[i];
  return s;
}

static struct P scale(struct P p, ld k) {
  p.a *= k;
  p.b *= k;
  return p;
}

static ld apply_all(ld (*fs[])(ld), int n, ld x) {
  for (int i = 0; i < n; i++) x = fs[i](x);
  return x;
}

static ld inc(ld x) { return x + 1; }
static ld dbl(ld x) { return x * 2; }
static ld neg(ld x) { return -x; }

int main(void) {
  ld local[3] = {1.5L, 2, 3.25f};
  struct P p = {1.5L, 2.5L};
  struct N n = {7, {0.5L, 0.25L}, {10, 20}};
  struct N *np = &n;
  printf("%.4Lf %.4Lf\n", sum(local, 3), local[1]);
  printf("%.4Lf %.4Lf %d\n", p.a, p.b, n.tag);
  np->p.a += 1;
  np->arr[1] *= 3;
  np->p.b++;
  --np->p.b;
  printf("%.4Lf %.4Lf %.4Lf\n", np->p.a, np->p.b, np->arr[1]);
  struct P q = scale(p, 4);
  printf("%.4Lf %.4Lf\n", q.a, q.b);
  struct P *pp = (struct P *)malloc(2 * sizeof(struct P));
  pp[0] = p;
  pp[1] = scale(pp[0], 0.5L);
  printf("%.4Lf %.4Lf %d\n", pp[1].a, pp[1].b, (int)sizeof(struct P));
  free(pp);
  ld (*fs[3])(ld) = {inc, dbl, neg};
  printf("%.4Lf\n", apply_all(fs, 3, 3.5L));
  union V v;
  v.x = -2.5L;
  printf("%llx %x\n", v.r.m, (unsigned)v.r.se);
  v.r.m = 0xC000000000000000ULL;
  v.r.se = 0x3FFF;
  printf("%.4Lf\n", v.x);
  ld *heap = (ld *)calloc(4, sizeof(ld));
  for (int i = 0; i < 4; i++) heap[i] = i * 0.5L + 0.125L;
  ld acc = 0;
  for (ld *it = heap; it < heap + 4; it++) acc += *it;
  printf("%.4Lf %d\n", acc, (int)(heap + 3 - heap));
  memcpy(heap + 1, heap, sizeof(ld));
  printf("%.4Lf\n", heap[1]);
  free(heap);
  ld t = (p.a > p.b) ? p.a : p.b;
  ld u = (p.a < p.b, p.b);
  printf("%.4Lf %.4Lf\n", t, u);
  int i = 3;
  ld w = i > 2 ? 1 : 2.5L;
  printf("%.4Lf\n", w);
  ld big = 1;
  for (int k = 0; k < 100; k++) big *= 10;
  printf("%.6Le %.6Le\n", big, 1 / big);
  return 0;
}
