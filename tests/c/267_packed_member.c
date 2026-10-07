#include <stdio.h>
#include <stddef.h>
#include <string.h>

typedef struct { unsigned long bits[2]; } Mask;
struct Sig { int flags; Mask mask __attribute__((__packed__)); };
struct Mix { char c; long l __attribute__((packed)); short s; int i __attribute__((packed)); };
struct Plain { char c; long l; short s; int i; };
struct Nest { char c; struct Sig in __attribute__((packed)); char tail; };
struct Arr { char c; unsigned short v[3] __attribute__((packed)); long w; };

int main(void)
{
  printf("%d %d %d %d %d\n", (int)sizeof(struct Sig), (int)sizeof(struct Mix), (int)sizeof(struct Plain), (int)sizeof(struct Nest), (int)sizeof(struct Arr));
  printf("%d %d %d %d %d\n", (int)_Alignof(struct Sig), (int)_Alignof(struct Mix), (int)_Alignof(struct Nest), (int)_Alignof(struct Arr), (int)_Alignof(Mask));
  printf("%d %d %d %d %d\n", (int)offsetof(struct Sig, mask), (int)offsetof(struct Mix, l), (int)offsetof(struct Mix, s), (int)offsetof(struct Mix, i), (int)offsetof(struct Nest, in));
  printf("%d %d %d\n", (int)offsetof(struct Nest, tail), (int)offsetof(struct Arr, v), (int)offsetof(struct Arr, w));
  struct Sig s;
  memset(&s, 0, sizeof s);
  s.flags = 7;
  s.mask.bits[0] = 0x1122334455667788UL;
  s.mask.bits[1] = 0x99aabbccddeeff00UL;
  printf("%d %lx %lx\n", s.flags, s.mask.bits[0], s.mask.bits[1]);
  struct Mix m;
  memset(&m, 0, sizeof m);
  m.c = 1;
  m.l = 0x0102030405060708L;
  m.s = 0x0a0b;
  m.i = 0x0c0d0e0f;
  printf("%d %lx %x %x\n", m.c, m.l, m.s, m.i);
  unsigned char *p = (unsigned char *)&m;
  for (int k = 0; k < (int)sizeof m; k++) printf("%02x", p[k]);
  printf("\n");
  struct Nest n;
  memset(&n, 0, sizeof n);
  n.c = 'a';
  n.in.flags = -5;
  n.in.mask.bits[1] = 42;
  n.tail = 'z';
  printf("%c %d %lu %c\n", n.c, n.in.flags, n.in.mask.bits[1], n.tail);
  struct Arr a;
  memset(&a, 0, sizeof a);
  a.c = 'q';
  a.v[0] = 1;
  a.v[2] = 3;
  a.w = 99;
  printf("%c %d %d %ld\n", a.c, a.v[0], a.v[2], a.w);
  return 0;
}
