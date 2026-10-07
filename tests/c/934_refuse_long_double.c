/* refused: long double */
#include <stdio.h>
int main(void){ long double x = 1.5L; x = x * 2; printf("%d\n", (int)x); return 0; }
