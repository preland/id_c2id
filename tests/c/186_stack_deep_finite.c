#include <stdio.h>
static long down(long n) {
    char pad[4096];
    pad[n & 4095] = (char)n;
    if (n == 0) return 0;
    return 1 + pad[n & 4095] * 0 + down(n - 1);
}
int main(void) {
    printf("%ld\n", down(12000));
    return 0;
}
