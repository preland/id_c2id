#include <stdio.h>
#include <stdlib.h>
static int f(int a) { int v = ({ if (a) return 5; 3; }); return v + 100; }
static int g(int a) { int n = 0; for (int i = 0; i < 6; i++) { n += ({ if (i == a) continue; if (i == 4) break; i; }); } return n; }
struct P { int a, b; };
int main(void) {
  struct P p = ({ struct P q = {1, 2}; q; });
  int t = ({ int u = 5; u; }) > 3 && ({ int w = 0; w; }) == 0;
  int m = (1 ? ({ 4; }) : ({ exit(3); 5; }));
  printf("%d %d %d %d %d %d %d\n", f(1), f(0), g(2), p.a + p.b, t, m, ({ 1; }) ? ({ 7; }) : 8);
  int r = ({ if (m) exit(4); 1; });
  printf("not reached %d\n", r);
  return 0;
}
