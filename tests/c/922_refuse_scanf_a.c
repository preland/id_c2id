#include <stdio.h>
int main(void) {
  double d;
  int r = sscanf("0x1p3", "%a", &d);
  printf("%d\n", r);
  return 0;
}
