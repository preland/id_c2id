#include <stdio.h>
struct __attribute__((packed)) Z { char c; int : 0; int i; };
int main(void) { struct Z z; z.c = 1; z.i = 2; printf("%d %d\n", (int)sizeof z, z.i); return 0; }
