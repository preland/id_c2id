#include <stdio.h>
int main(void) {
  char w[16];
  int r = sscanf("abc123", "%[a-z]", w);
  printf("%d %s\n", r, w);
  return 0;
}
