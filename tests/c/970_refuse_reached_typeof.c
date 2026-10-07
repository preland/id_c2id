#include <stdio.h>
static int pick(int a, long b)
{
  return __builtin_choose_expr(__builtin_types_compatible_p(typeof(a), long), 1, 2) + (int)b;
}
int main(void) { printf("%d\n", pick(1, 2)); return 0; }
