/* refused: __atomic_load_n (GCC atomic builtins) not provided */
#include <stdio.h>
int main(void){ unsigned long v = 5, o = __atomic_load_n(&v, __ATOMIC_RELAXED); printf("%lu\n", o); return 0; }
