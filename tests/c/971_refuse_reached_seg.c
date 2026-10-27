#include <stdio.h>
int __seg_gs per_cpu_counter;
static int read_counter(int enabled)
{
  if (enabled) return per_cpu_counter;
  return -1;
}
int main(void) { printf("%d\n", read_counter(0)); return 0; }
