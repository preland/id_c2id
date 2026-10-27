#include <stdio.h>
#include <stdlib.h>
static void first(void) { puts("first registered, runs last"); }
static void second(void) { puts("second registered, runs first"); }
int main(void) {
  const char *home = getenv("HOME");
  const char *none = getenv("C2ID_NO_SUCH_VARIABLE_X");
  atexit(first);
  atexit(second);
  printf("%d %d\n", home != 0 && home[0] == '/', none == 0);
  if (home) exit(3);
  return 0;
}
