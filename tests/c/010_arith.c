// Integer arithmetic, precedence, and 32-bit wraparound.
#include <stdio.h>
int main(void) {
    int a = 17, b = 5;
    printf("%d %d %d %d %d\n", a + b, a - b, a * b, a / b, a % b);
    printf("%d %d\n", -a / b, -a % b);          /* C99 truncation toward zero */
    unsigned int u = 4000000000u;
    printf("%u %u\n", u + u, u * 3u);           /* defined wraparound */
    int w = 2147483647;
    printf("%d\n", (int)((unsigned)w + 1u));
    printf("%d\n", (a > b) + (a < b) * 10 + (a == 17) * 100);
    return 0;
}
