#include <stdio.h>
static long down(long n) {
    char pad[4096];
    pad[n & 4095] = (char)n;
    if (n == -1) return 0;
    return pad[n & 4095] + down(n + 1);
}
int main(void) {
    printf("start\n");
    printf("%ld\n", down(0));
    return 0;
}
