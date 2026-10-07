#include <stdio.h>
struct P { int x, y; };
struct W { struct P a, b; int z; };
struct P make(int a, int b) { struct P p; p.x = a; p.y = b; return p; }
struct P (*ops[2])(int, int) = { make, make };
struct P glob;
struct P gmake(void) { return glob; }
int main(void) {
  struct W w = { make(1, 2), make(3, 4), 5 };
  struct P arr[2] = { make(5, 6), make(7, 8) };
  struct P q;
  int c = 1;
  q = c ? make(1, 1) : make(2, 2);
  printf("%d %d %d %d %d\n", w.a.x, w.a.y, w.b.x, w.b.y, w.z);
  printf("%d %d %d %d\n", arr[0].x, arr[0].y, arr[1].x, arr[1].y);
  printf("%d\n", q.x);
  printf("%d\n", (make(1, 2), make(3, 4)).x);
  printf("%d\n", ops[1](9, 8).x);
  glob = make(4, 5);
  printf("%d\n", gmake().y);
  return 0;
}
