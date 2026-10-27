/* refused: _Alignas on a local declaration is treated as an undefined function call */
#include <stdio.h>
int main(void){
  _Alignas(16) unsigned long long acc[8] = {1,2,3,4,5,6,7,8};
  printf("%llu\n", acc[3]);
  return 0;}
