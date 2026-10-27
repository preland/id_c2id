#include <stdio.h>
int main(void)
{
  __int128 big = 1;
  big <<= 100;
  double d = (double)big;
  printf("%g\n", d);
  return 0;
}
