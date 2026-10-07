int printf(const char *f, ...);
struct P { int x, y; };
int g[8] = { 1, [2 ... 4] = 7, [6] = 9 };
struct P ps[4] = { [0 ... 2] = { 1, 2 }, [3] = { .y = 5 } };
int main(void) {
  int loc[6] = { [1 ... 3] = 4, [5] = 8 };
  int i, s = 0;
  for (i = 0; i < 8; i++) s = s * 3 + g[i];
  for (i = 0; i < 6; i++) s = s * 3 + loc[i];
  printf("%d %d %d %d %d\n", s, ps[1].y, ps[2].x, ps[3].x, ps[3].y);
  return 0;
}
