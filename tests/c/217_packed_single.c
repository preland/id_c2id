#include <stdio.h>
typedef struct { unsigned short u16; } __attribute__((packed)) Unal16;
typedef union { unsigned short u16; unsigned int u32; unsigned long uArch; } __attribute__((packed)) UnalAny;
struct __attribute__((packed)) One { long v; };
int main(void) {
  unsigned char buf[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
  Unal16 *p = (Unal16 *)(buf + 1);
  UnalAny *q = (UnalAny *)(buf + 3);
  struct One *o = (struct One *)(buf + 5);
  printf("%u %u %lu %ld\n", p->u16, q->u32, q->uArch, o->v);
  printf("%d %d %d\n", (int)sizeof(Unal16), (int)sizeof(UnalAny), (int)sizeof(struct One));
  return 0;
}
