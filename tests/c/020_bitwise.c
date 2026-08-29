// Bitwise operators, including the signed/unsigned shift distinction.
#include <stdio.h>
int main(void) {
    unsigned int x = 0xdeadbeefu;
    printf("%x %x %x %x\n", x & 0xffffu, x | 0xf0000000u, x ^ 0xffffffffu, ~x);
    printf("%x %x\n", x >> 4, x << 4);
    int s = -16;
    printf("%d %d\n", s >> 2, (int)((unsigned)s >> 2));  /* arithmetic vs logical */
    unsigned long long big = 0x0123456789abcdefULL;
    printf("%llx %llx\n", big >> 32, (big & 0xffffffffULL) << 8);
    return 0;
}
