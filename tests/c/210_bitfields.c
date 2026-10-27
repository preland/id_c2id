#include <stdio.h>
#include <string.h>
struct A { unsigned a : 3; unsigned b : 5; unsigned c : 8; unsigned d : 16; };
struct B { int s : 4; unsigned u : 4; char c; short h : 9; long long big : 40; };
struct C { unsigned char x : 1; unsigned char y : 7; unsigned char z : 3; int w : 20; _Bool f : 1; _Bool g : 1; };
struct D { char pad; int a : 12; int b : 12; int : 0; int c : 5; };
struct E { unsigned long l : 60; unsigned long m : 4; int i : 31; int j : 1; };
union U { struct { unsigned lo : 8; unsigned hi : 8; } p; unsigned short whole; };
struct F { int tag; struct A inner; unsigned last : 3; };
static struct A ga = { 5, 17, 200, 40000 };
static struct B gb = { -3, 9, 'q', -200, -123456789012LL };
static unsigned pick(struct A *p) { return p->b + p->d; }
static int neg(struct B b) { return b.s * 100 + b.u; }
int main(void) {
  struct A a; struct B b; struct C c; struct D d; struct E e; union U u; struct F f;
  struct A arr[3];
  printf("%d %d %d %d %d %d\n", (int)sizeof a, (int)sizeof b, (int)sizeof c, (int)sizeof d, (int)sizeof e, (int)sizeof f);
  memset(&a, 0, sizeof a); a.a = 7; a.b = 31; a.c = 255; a.d = 65535;
  printf("%u %u %u %u\n", a.a, a.b, a.c, a.d);
  a.a = 9; a.b = 33; a.c = 300; a.d = 70000;
  printf("%u %u %u %u\n", a.a, a.b, a.c, a.d);
  a.a += 5; a.b++; a.c--; a.d <<= 1; a.a ^= 3;
  printf("%u %u %u %u %u\n", a.a, a.b, a.c, a.d, pick(&a));
  printf("%d %d %d %d\n", a.a - 6, (int)(a.a - 6u), a.a > 1, -a.a);
  memset(&b, 0, sizeof b);
  b.s = -8; b.u = 15; b.c = 'x'; b.h = -256; b.big = -1099511627776LL;
  printf("%d %u %c %d %lld\n", b.s, b.u, b.c, b.h, b.big);
  b.s = 9; b.h = 300; b.big = 1LL << 39;
  printf("%d %d %lld %d\n", b.s, b.h, b.big, neg(b));
  b.s--; b.s--; b.u += 20; b.big += 7;
  printf("%d %u %lld\n", b.s, b.u, b.big);
  memset(&c, 0xff, sizeof c);
  c.x = 0; c.z = 5; c.w = -1; c.f = 0; c.g = 2;
  printf("%u %u %u %d %d %d\n", c.x, c.y, c.z, c.w, c.f, c.g);
  memset(&d, 0, sizeof d);
  d.pad = 'p'; d.a = -2048; d.b = 2047; d.c = 15;
  printf("%c %d %d %d\n", d.pad, d.a, d.b, d.c);
  memset(&e, 0, sizeof e);
  e.l = 0xfffffffffffffffUL; e.m = 9; e.i = -1073741824; e.j = -1;
  printf("%lx %lu %d %d\n", (unsigned long)e.l, (unsigned long)e.m, e.i, e.j);
  u.whole = 0; u.p.lo = 0x12; u.p.hi = 0x34;
  printf("%x %u %u\n", u.whole, u.p.lo, u.p.hi);
  f.tag = 99; f.inner = ga; f.last = 6; f.inner.a = 1;
  printf("%d %u %u %u %u\n", f.tag, f.inner.a, f.inner.d, f.last, ga.a);
  printf("%u %u %u %u | %d %u %c %d %lld\n", ga.a, ga.b, ga.c, ga.d, gb.s, gb.u, gb.c, gb.h, gb.big);
  for (int i = 0; i < 3; i++) { arr[i].a = i; arr[i].b = i * 11; arr[i].c = i * 100; arr[i].d = i * 30000; }
  for (int i = 0; i < 3; i++) printf("%u,%u,%u,%u ", arr[i].a, arr[i].b, arr[i].c, arr[i].d);
  printf("\n");
  { struct A *p = &arr[2]; p->a = 4; p->d = p->d + p->b; printf("%u %u\n", p->a, p->d); }
  { struct D dd = { 'z', 5, -6, 7 }; struct A aa = { .c = 9, .a = 2 }; struct C cc = { 1, 100, 6, -500, 1, 0 };
    printf("%c %d %d %d | %u %u %u %u | %u %u %u %d %d %d\n", dd.pad, dd.a, dd.b, dd.c, aa.a, aa.b, aa.c, aa.d, cc.x, cc.y, cc.z, cc.w, cc.f, cc.g); }
  { struct A x1, x2; memset(&x1, 0xa5, sizeof x1); x2 = x1; printf("%u %u %u %u\n", x2.a, x2.b, x2.c, x2.d); }
  return 0;
}
