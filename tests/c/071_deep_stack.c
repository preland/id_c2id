// A stack far deeper than its first chunk, frames of unequal size, and a call
// that goes back and forth across a chunk boundary.
#include <stdio.h>
static long sum(int n) {
    if (n == 0) return 0;
    return n + sum(n - 1);
}
static long fat(int n, long acc) {
    char pad[200];
    pad[n & 127] = (char)n;
    if (n == 0) return acc;
    return fat(n - 1, acc + pad[n & 127]);
}
static int leaf(int x) { return x * 3 + 1; }
static long bounce(int n, int rounds) {
    long t = 0;
    if (n > 0) return bounce(n - 1, rounds);
    for (int i = 0; i < rounds; i++) t += leaf(i);
    return t;
}
int main(void) {
    printf("%ld\n", sum(100000));
    printf("%ld\n", fat(5000, 0));
    printf("%ld\n", bounce(20000, 1000));
    return 0;
}
