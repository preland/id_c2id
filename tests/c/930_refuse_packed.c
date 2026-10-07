#include <stdio.h>
struct __attribute__((packed)) P { char c; int i; };
int main(void) { struct P p; p.c = 1; p.i = 2; printf("%d %d\n", (int)sizeof p, p.i); return 0; }
