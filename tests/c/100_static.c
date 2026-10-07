#include <stdio.h>

static int g = 3;

int counter(void) { static int n = 5; return ++n; }
int bump(int i) { static int a[3]; a[i]++; return a[i]; }
const char *name(int i) { static const char *names[] = {"zero", "one", "two"}; return names[i]; }
int table(int i) { static const int t[] = {10, 20, 30, 40}; return t[i]; }
int scoped(void) { static int n = 100; { static int n = 7; n++; } return ++n; }
int *where(void) { static int cell = 42; return &cell; }
int items(void) { static const int t[] = {5, 6, 7, 8, 9}; return (int)(sizeof t / sizeof t[0]) * 10 + (int)sizeof(t[0]); }
int plain(int x) { int local = 1; local += x; return local; }

int main(void) {
  int i, r1, r2;
  counter();
  counter();
  printf("%d\n", counter());
  bump(1);
  bump(1);
  printf("%d %d\n", bump(1), bump(0));
  for (i = 0; i < 3; i++) printf("%s %d\n", name(i), table(i + 1));
  r1 = scoped();
  r2 = scoped();
  printf("%d %d %d\n", r1, r2, g);
  printf("%d\n", items());
  *where() += 1;
  printf("%d %d\n", *where(), plain(4) + plain(4));
  return 0;
}
