int printf(const char *f, ...);
int calls;
int f(int x) { calls++; return x; }
int main(void) {
  int a = 3, z = 0, b, r1, r2, c1, c2;
  b = a ?: 7;
  r1 = f(5) ?: f(6);
  c1 = calls;
  r2 = f(0) ?: f(8);
  c2 = calls;
  printf("%d %d %d %d %d %d %d\n", b, z ?: 9, r1, c1, r2, c2, a ?: z);
  if (z ?: f(0)) printf("no\n"); else printf("else\n");
  long l = z ?: 100000000000L;
  char *p = 0; char *q = "q";
  printf("%ld %s %d\n", l, p ?: q, (a ?: z) + (z ?: a));
  return 0;
}
