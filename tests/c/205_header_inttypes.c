/* preprocessor stops: no inttypes.h in c2id/include */
#include <inttypes.h>
#include <stdio.h>
int main(void){ uint64_t v = 1; printf("%d\n", (int)v); return 0; }
