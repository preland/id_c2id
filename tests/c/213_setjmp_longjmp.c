#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
static jmp_buf top, inner;
static int depth_reached;
static void unwind(int n, int code) { depth_reached = n; if (n == 0) longjmp(top, code); unwind(n - 1, code); depth_reached = -1; }
static int try_parse(const char *s) {
  jmp_buf here;
  int r = setjmp(here);
  if (r == 0) {
    while (*s) { if (*s < '0' || *s > '9') longjmp(here, (int)*s); s++; }
    return 0;
  }
  return r;
}
static volatile int counter;
static int loop_retry(void) {
  jmp_buf jb;
  volatile int tries = 0;
  setjmp(jb);
  tries++;
  if (tries < 4) longjmp(jb, 1);
  return tries;
}
static int sw(int mode) {
  switch (setjmp(inner)) {
  case 0: if (mode) longjmp(inner, mode); return 100;
  case 1: return 101;
  case 2: return 102;
  default: return -1;
  }
}
static int sum_va(int n, ...) { return n; }
static void deep_va(int n) { if (n == 0) longjmp(top, 9); sum_va(n, 1, 2, 3); deep_va(n - 1); }
int main(void) {
  int r = setjmp(top);
  if (r == 0) { printf("start\n"); unwind(50, 5); printf("not reached\n"); }
  else printf("back %d depth %d\n", r, depth_reached);
  printf("%d %d %d\n", try_parse("1234"), try_parse("12x4"), try_parse("a"));
  printf("%d\n", loop_retry());
  printf("%d %d %d %d\n", sw(0), sw(1), sw(2), sw(7));
  r = setjmp(top);
  if (r == 0) deep_va(40);
  printf("deep %d\n", r);
  for (volatile int i = 0; i < 2000; i++) { if (setjmp(top) == 0) unwind(5, 3); counter++; }
  printf("%d\n", counter);
  if (setjmp(top) == 0) longjmp(top, 0);
  else printf("zero became nonzero\n");
  { char buf[8] = "ok"; if (setjmp(top) == 0) exit(4); printf("%s\n", buf); }
  return 0;
}
