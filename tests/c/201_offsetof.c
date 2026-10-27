/* refused: offsetof expands to __builtin_offsetof, not provided */
#include <stdio.h>
#include <stddef.h>
struct S { char c; long x; };
int main(void){ printf("%d\n", (int)offsetof(struct S, x)); return 0; }
