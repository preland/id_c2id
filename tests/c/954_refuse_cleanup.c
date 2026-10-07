#include <stdio.h>
static void f(int *p) { printf("f(%d)\n", *p); }
int main(void) {
  for (int x __attribute__((cleanup(f))) = 0; x < 2; x++) printf("body %d\n", x);
  return 0;
}
