/* refused: C11 <stdatomic.h> is not provided (the GCC __atomic and __sync builtins are: 225_atomic_builtins.c) */
#include <stdatomic.h>
#include <stdio.h>
int main(void) { atomic_int v = 5; printf("%d\n", atomic_load(&v)); return 0; }
