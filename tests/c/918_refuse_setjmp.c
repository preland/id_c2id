#include <stdio.h>
#include <setjmp.h>
static jmp_buf env;
int main(void) {
  if (setjmp(env) == 0) longjmp(env, 1);
  printf("back\n");
  return 0;
}
