#include <stdio.h>
#include <string.h>

struct Rec { char tag; char name[4]; };

static int classify(char c)
{
  if (c < 0) return 1;
  if (c > 100) return 2;
  return 0;
}

static int widen(const char *p)
{
  return *p + 1;
}

int main(void)
{
  char hi = (char)200;
  char lo = 'A';
  char arr[3] = { (char)255, (char)128, 5 };
  struct Rec r;
  memset(&r, 0, sizeof r);
  r.tag = (char)250;
  r.name[1] = (char)129;
  printf("%d %d %d\n", hi, lo, hi > lo);
  printf("%d %d %d\n", classify(hi), classify(lo), classify((char)127));
  printf("%d %d %d %d\n", arr[0], arr[1], arr[2], widen(&arr[0]));
  printf("%d %d %d\n", r.tag, r.name[1], r.tag + r.name[1]);
  hi = hi + 100;
  printf("%d %d\n", hi, (hi >> 1) + (signed char)hi);
  printf("%d %d\n", (int)sizeof(char), (unsigned char)hi == hi);
  return 0;
}
