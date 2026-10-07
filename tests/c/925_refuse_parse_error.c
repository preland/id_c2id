#include <stdio.h>
int main(void) {
  void *p = &&done;
  goto *p;
done:
  printf("done\n");
  return 0;
}
