#include <stdio.h>

extern int missing_cb(int);
static int (*unused_table[])(int) = { missing_cb };
static int plain_static(int x) { return x + 100; }
extern inline __attribute__((__gnu_inline__)) int gnu_only(int x) { return x * 2 + missing_cb(x); }
static int (*chosen)(int) = plain_static;

static int apply2(int (*f)(int, int), int a, int b)
{
  if (!f) return -1;
  return f(a, b);
}

int main(void)
{
  printf("%d %d\n", chosen(5), apply2(0, 1, 2));
  return 0;
}
