#include <stdio.h>
struct P { int x, y; };
int show(struct P *p) { return p->x + p->y; }
int main(void) {
  struct P p = {1, 2};
  int (*fn)(struct P *) = show;
  int (*bad)(struct P) = (int (*)(struct P))show;
  printf("%d %d\n", fn(&p), bad(p));
  return 0;
}
