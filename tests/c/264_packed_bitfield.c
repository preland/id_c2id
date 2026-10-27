#include <stdio.h>
#include <stddef.h>
#include <string.h>

struct __attribute__((packed)) G { unsigned short limit0; unsigned short base0; unsigned char base1; unsigned short type : 4, s : 1, dpl : 2, p : 1; unsigned short limit1 : 4, avl : 1, l : 1, d : 1, g : 1; unsigned char base2; };
struct __attribute__((packed)) C { char a; unsigned int x : 12; unsigned int y : 12; unsigned int z : 12; char tail; };
struct __attribute__((packed)) S { unsigned char lead; int v : 20; unsigned long long w : 40; short q : 3; };
struct __attribute__((packed)) B { unsigned char b0 : 3; unsigned int b1 : 30; unsigned char b2 : 7; };
struct __attribute__((packed)) N { char pad; struct C inner; signed char t : 5; };
struct __attribute__((packed)) E { unsigned char a : 1; unsigned int big : 32; unsigned long m : 33; };

static unsigned pick(const struct C *c)
{
  return c->x + c->y * 2 + c->z * 3;
}

int main(void)
{
  printf("%d %d %d %d %d %d\n", (int)sizeof(struct G), (int)sizeof(struct C), (int)sizeof(struct S), (int)sizeof(struct B), (int)sizeof(struct N), (int)sizeof(struct E));
  printf("%d %d %d\n", (int)offsetof(struct G, base2), (int)offsetof(struct C, tail), (int)offsetof(struct N, inner));
  struct G g;
  memset(&g, 0, sizeof g);
  g.limit0 = 0xffff;
  g.base1 = 0x5a;
  g.type = 11;
  g.s = 1;
  g.dpl = 3;
  g.p = 1;
  g.limit1 = 9;
  g.avl = 1;
  g.l = 0;
  g.d = 1;
  g.g = 1;
  g.base2 = 0xc3;
  printf("%x %x %u %u %u %u %u %u %u %u %u %x\n", g.limit0, g.base1, g.type, g.s, g.dpl, g.p, g.limit1, g.avl, g.l, g.d, g.g, g.base2);
  unsigned char *raw = (unsigned char *)&g;
  for (int k = 0; k < (int)sizeof g; k++) printf("%02x", raw[k]);
  printf("\n");

  struct C c;
  memset(&c, 0, sizeof c);
  c.a = 'a';
  c.x = 0xabc;
  c.y = 0x123;
  c.z = 0xfff;
  c.tail = 't';
  printf("%c %x %x %x %c %u\n", c.a, c.x, c.y, c.z, c.tail, pick(&c));
  c.x++;
  c.y += 0x1000;
  c.z = c.x + c.y;
  printf("%x %x %x\n", c.x, c.y, c.z);
  raw = (unsigned char *)&c;
  for (int k = 0; k < (int)sizeof c; k++) printf("%02x", raw[k]);
  printf("\n");

  struct S s;
  memset(&s, 0xff, sizeof s);
  s.lead = 1;
  s.v = -12345;
  s.w = 0xabcdef1234ULL;
  s.q = -2;
  printf("%u %d %llx %d\n", s.lead, s.v, (unsigned long long)s.w, s.q);
  s.v = 524287;
  s.w >>= 4;
  s.q = 3;
  printf("%d %llx %d\n", s.v, (unsigned long long)s.w, s.q);

  struct B b;
  memset(&b, 0, sizeof b);
  b.b0 = 5;
  b.b1 = 0x2aaaaaaa;
  b.b2 = 100;
  printf("%u %x %u\n", b.b0, b.b1, b.b2);
  b.b1 ^= 0x15555555;
  b.b0 = 2;
  printf("%u %x %u\n", b.b0, b.b1, b.b2);

  struct N n;
  memset(&n, 0, sizeof n);
  n.pad = 'p';
  n.inner.a = 'i';
  n.inner.x = 1;
  n.inner.y = 2;
  n.inner.z = 3;
  n.inner.tail = 'j';
  n.t = -7;
  printf("%c %c %u %u %u %c %d\n", n.pad, n.inner.a, n.inner.x, n.inner.y, n.inner.z, n.inner.tail, n.t);

  struct E e;
  memset(&e, 0, sizeof e);
  e.a = 1;
  e.big = 0xdeadbeefu;
  e.m = 0x1ffffffffUL;
  printf("%u %x %lx\n", e.a, e.big, (unsigned long)e.m);
  struct C arr[3];
  memset(arr, 0, sizeof arr);
  for (int k = 0; k < 3; k++) {
    arr[k].x = k + 1;
    arr[k].y = (k + 1) * 100;
    arr[k].z = (k + 1) * 1000;
  }
  printf("%u %u %u\n", pick(&arr[0]), pick(&arr[1]), pick(&arr[2]));
  return 0;
}
