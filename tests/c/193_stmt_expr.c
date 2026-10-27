#include <stdio.h>
#define S(lit) ({ static const struct { unsigned long n; char s[sizeof(lit)]; } l_ = { sizeof(lit) - 1, lit }; (char*)l_.s; })
#define SQ(x) ({ int q_ = (x); q_ * q_; })
static int calls;
static int bump(void) { return ++calls; }
static void note(int v) { printf("note %d\n", v); }
int main(void) {
  int x = ({ int t = 3; t * 2; });
  int y = ({ int s = 0; for (int i = 1; i <= 4; i++) s += i; s; });
  int z = ({ int r; if (x > 5) r = 10; else r = 20; r; }) + ({ int u = 1; u; });
  int w = SQ(bump()) + SQ(bump());
  char *p = S("hello");
  long big = ({ long a = 1; a <<= 40; a + 5; });
  double d = ({ double h = 1.5; h * 3; });
  int k = 0;
  while (({ k++; k < 4; })) { }
  ({ note(7); });
  ({ int e = 1; (void)e; });
  int nest = ({ int a = ({ int b = 2; b + 1; }); a * 10; });
  printf("%d %d %d %d %s %ld %.1f %d %d %d\n", x, y, z, w, p, big, d, k, calls, nest);
  return ({ int c = 3; c - 3; });
}
