#include <stdio.h>
struct P { int x, y; };
int shift(struct P p) { p.x = 100; return p.x + p.y; }
int main(void) {
  struct P p = {1, 2};
  printf("%d %d\n", shift(p), p.x);
  return 0;
}
