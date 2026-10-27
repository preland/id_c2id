#include <stdio.h>
struct P { int x, y; };
int main(void) {
  struct P p = {1, 2};
  printf("%d\n", p);
  return 0;
}
