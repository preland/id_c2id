#include <stdio.h>
struct P { int x, y; };
int main(void) {
  struct P p = (struct P){1, 2};
  printf("%d\n", p.y);
  return 0;
}
