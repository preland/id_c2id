#include <stdio.h>
#include <setjmp.h>
typedef sigjmp_buf Err;
static Err err;
static int risky(int n) { if (n > 2) siglongjmp(err, n * 10); return n; }
static int guarded(int n) {
  int v = sigsetjmp(err, 1);
  if (v) return -v;
  return risky(n) + 100;
}
int main(void) {
  for (int i = 1; i <= 4; i++) printf("%d ", guarded(i));
  printf("\n");
  return 0;
}
