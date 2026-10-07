#include <stdio.h>
static long sum(int n) { return n == 0 ? 0 : n + sum(n - 1); }
static long arms(int c, int n) { return c == 0 ? 0 : n + (long)n; }
static long wide(int c, int n) { return c ? n : 3000000000L; }
static unsigned uarm(int c, unsigned u, int n) { return c ? u : n; }
static long ulong_arm(int c, unsigned u) { return c ? u : -1; }
static long mixed(int c, int n) { return (c ? n : 1u) - 2; }
static long shortarm(int c, short s, char ch) { return (c ? s : ch) * 100000; }
static int *ptrarm(int c, int *p) { return c ? 0 : p; }
int main(void) {
    int big = 2000000000;
    int v = 5;
    printf("%ld\n", sum(100000));
    printf("%ld\n", arms(1, big));
    printf("%ld\n", wide(1, big));
    printf("%ld\n", wide(0, big));
    printf("%u\n", uarm(0, 7u, -1));
    printf("%u\n", uarm(1, 7u, -1));
    printf("%ld\n", ulong_arm(0, 5u));
    printf("%ld\n", mixed(0, -5));
    printf("%ld\n", mixed(1, -5));
    printf("%ld\n", shortarm(1, 30000, 1));
    printf("%d\n", ptrarm(1, &v) == 0);
    printf("%d\n", *ptrarm(0, &v));
    printf("%ld\n", (long)(big ? big + big : 0L));
    return 0;
}
