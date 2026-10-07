/* refused: __builtin_prefetch not provided */
#include <stdio.h>
int main(void){
  int a[4] = {1,2,3,4};
  __builtin_prefetch(&a[1], 0, 3);
  printf("%d\n", a[2]);
  return 0;}
