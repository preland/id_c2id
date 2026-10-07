#include <stdio.h>
static int n;
static int next(void) { return ++n; }
int main(void) {
  int i = 0, k;
  switch (next()) { case 5: puts("five"); break; case 1: puts("one"); break; default: puts("other"); }
  switch (i++) { case 3: puts("three"); break; case 0: puts("zero"); break; default: puts("d"); }
  switch (next() + next()) { case 2: puts("two"); break; case 7: puts("seven"); break; case 6: puts("six"); break; }
  switch ((k = 4) && next()) { case 0: puts("zero"); break; case 1: puts("true"); break; }
  switch (i > 0 ? next() : 99) { case 99: puts("ninety-nine"); break; case 5: puts("five again"); break; default: puts("sw"); }
  printf("%d %d %d\n", n, i, k);
  return 0;
}
