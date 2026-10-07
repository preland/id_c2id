int printf(const char *f, ...);
int cls(int c) {
  switch (c) {
  case 'a' ... 'f': return 1;
  case '0' ... '9':
  case 'x': return 2;
  case -3 ... -1: return 3;
  case 100 ... 99: return 99;
  default: return 0;
  }
}
int fall(int a) {
  int r = 0;
  switch (a) {
  case 1 ... 3: r += 1;
  case 4 ... 5: r += 10; break;
  case 6: r = 7;
  }
  return r;
}
int main(void) {
  int i, s = 0;
  for (i = -5; i < 130; i++) s = s * 3 + cls(i);
  printf("%d %d %d %d %d\n", s, fall(2), fall(4), fall(6), fall(9));
  printf("%d %d %d\n", cls('c'), cls('7'), cls(-2));
  return 0;
}
