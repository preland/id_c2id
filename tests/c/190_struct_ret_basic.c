#include <stdio.h>
struct P { int x, y; };
struct P make(int a, int b) { struct P p; p.x = a; p.y = b; return p; }
int main(void) {
  struct P p = make(3, 4);
  printf("%d %d\n", p.x, p.y);
  return 0;
}
