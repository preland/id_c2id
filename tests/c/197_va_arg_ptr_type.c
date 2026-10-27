/* misleading refusal: va_arg(ap, char *) derails the parser; reports 'if' and 'main' as undefined functions */
#include <stdio.h>
#include <stdarg.h>
static int strfmt(va_list ap) {
  char *s = va_arg(ap, char *);
  int n = va_arg(ap, int);
  if (!s) return -1;
  return n;
}
int main(void){
  printf("%d\n", 1);
  return 0;}
