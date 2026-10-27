#include <stdio.h>
#include <stddef.h>
#include <string.h>

struct __attribute__((packed)) P1 { char c; int i; };
struct P2 { char c; long l; short s; } __attribute__((packed));
typedef struct { unsigned char tag; unsigned int len; unsigned short crc; unsigned char data[5]; } __attribute__((packed)) Hdr;
struct __attribute__((__packed__)) P3 { char c; double d; float f; char *p; };
struct Plain { char c; int i; };
struct Mix { char a; struct P1 in; short tail; } __attribute__((packed));
struct Out { char a; struct P1 in; short tail; };
struct __attribute__((packed)) Anon { char a; struct { char x; long y; }; union { int u; char v[3]; }; };
union __attribute__((packed)) PU { char c; int i; char raw[5]; };
struct Al16 { char c; long long v; } __attribute__((packed, aligned(16)));
struct Al4 { char c; int i; } __attribute__((__packed__)) __attribute__((__aligned__(4)));
struct Al8 { char c; int i; } __attribute__((aligned(8)));
struct Arr { short n; long v[3]; } __attribute__((packed));

static int sum(const struct P1 *p, int n)
{
  int s = 0;
  for (int k = 0; k < n; k++) s += p[k].c * 1000 + p[k].i;
  return s;
}

int main(void)
{
  printf("%d %d %d %d %d %d\n", (int)sizeof(struct P1), (int)sizeof(struct P2), (int)sizeof(Hdr), (int)sizeof(struct P3), (int)sizeof(struct Plain), (int)sizeof(struct Mix));
  printf("%d %d %d %d\n", (int)sizeof(struct Out), (int)sizeof(struct Anon), (int)sizeof(union PU), (int)sizeof(struct Arr));
  printf("%d %d %d %d\n", (int)sizeof(struct Al16), (int)sizeof(struct Al4), (int)sizeof(struct Al8), (int)_Alignof(struct Al16));
  printf("%d %d %d %d %d %d\n", (int)_Alignof(struct P1), (int)_Alignof(struct P2), (int)_Alignof(Hdr), (int)_Alignof(struct Mix), (int)_Alignof(struct Al4), (int)_Alignof(struct Al8));
  printf("%d %d %d %d\n", (int)offsetof(struct P1, i), (int)offsetof(struct P2, l), (int)offsetof(struct P2, s), (int)offsetof(Hdr, crc));
  printf("%d %d %d %d %d\n", (int)offsetof(Hdr, data), (int)offsetof(struct P3, d), (int)offsetof(struct P3, f), (int)offsetof(struct P3, p), (int)offsetof(struct Mix, tail));
  printf("%d %d %d %d %d\n", (int)offsetof(struct Mix, in), (int)offsetof(struct Anon, y), (int)offsetof(struct Anon, u), (int)offsetof(struct Anon, v), (int)offsetof(struct Arr, v));
  printf("%d %d\n", (int)offsetof(struct Al4, i), (int)offsetof(struct Al8, i));

  struct P1 a;
  a.c = 'x';
  a.i = -123456789;
  printf("%c %d\n", a.c, a.i);
  struct P2 b;
  memset(&b, 0, sizeof b);
  b.c = 1;
  b.l = 0x0102030405060708L;
  b.s = -2;
  printf("%d %lx %d\n", b.c, b.l, b.s);
  unsigned char *raw = (unsigned char *)&b;
  for (int k = 0; k < (int)sizeof b; k++) printf("%02x", raw[k]);
  printf("\n");

  Hdr h;
  memset(&h, 0xee, sizeof h);
  h.tag = 7;
  h.len = 0xdeadbeefu;
  h.crc = 0xabcdu;
  h.data[0] = 1;
  h.data[4] = 5;
  printf("%u %x %x %u %u\n", h.tag, h.len, h.crc, h.data[0], h.data[4]);

  struct P3 d3;
  memset(&d3, 0, sizeof d3);
  d3.c = 'z';
  d3.d = 2.5;
  d3.f = -0.75f;
  d3.p = (char *)"hello";
  printf("%c %g %g %s\n", d3.c, d3.d, (double)d3.f, d3.p);

  struct P1 arr[4];
  for (int k = 0; k < 4; k++) {
    arr[k].c = (char)('a' + k);
    arr[k].i = k * 1000 + k;
  }
  printf("%d\n", sum(arr, 4));
  struct P1 *q = &arr[2];
  q->i += 10;
  q->c++;
  printf("%c %d %d\n", arr[2].c, arr[2].i, (int)((char *)&arr[3] - (char *)&arr[0]));
  arr[0] = arr[3];
  printf("%c %d\n", arr[0].c, arr[0].i);

  struct Mix m;
  memset(&m, 0, sizeof m);
  m.a = 1;
  m.in.c = 2;
  m.in.i = 0x11223344;
  m.tail = 0x5566;
  printf("%d %d %x %x\n", m.a, m.in.c, m.in.i, m.tail);
  struct P1 copy = m.in;
  printf("%d %x\n", copy.c, copy.i);

  struct Anon an;
  memset(&an, 0, sizeof an);
  an.a = 9;
  an.x = 8;
  an.y = 0x1122334455667788L;
  an.u = 0x01020304;
  printf("%d %d %lx %x %d\n", an.a, an.x, an.y, an.u, an.v[1]);

  union PU pu;
  memset(&pu, 0, sizeof pu);
  pu.i = 0x04030201;
  printf("%d %d %d\n", pu.c, pu.raw[3], pu.raw[4]);

  struct Arr ar;
  ar.n = 3;
  for (int k = 0; k < 3; k++) ar.v[k] = (long)k * 0x100000001L;
  printf("%d %lx %lx %lx\n", ar.n, ar.v[0], ar.v[1], ar.v[2]);

  struct Al16 a16;
  a16.c = 3;
  a16.v = 77;
  struct Al16 a16s[2];
  a16s[1].v = 5;
  printf("%d %lld %d\n", a16.c, a16.v, (int)((char *)&a16s[1] - (char *)&a16s[0]));
  return 0;
}
