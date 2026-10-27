#include <stdio.h>
static long widen(long x) { return x; }
static int narrow(int x) { return x; }
static unsigned char byte(unsigned char x) { return x; }
static unsigned uns(unsigned x) { return x; }
static long fromuns(unsigned x) { return x; }
static int back(long x) { return (int)x; }
static short sh(int x) { return x; }
int main(void) {
    unsigned u = 4000000000u;
    long big = 5000000000L;
    int neg = -3;
    printf("%ld\n", widen(neg));
    printf("%ld\n", widen(u));
    printf("%d\n", narrow(big));
    printf("%d\n", byte(300));
    printf("%u\n", uns(neg));
    printf("%ld\n", fromuns(neg));
    printf("%d\n", back(big + 1));
    printf("%d\n", sh(70000));
    printf("%ld\n", (long)(int)u);
    printf("%ld\n", (long)(unsigned)neg);
    printf("%d\n", (char)200);
    printf("%d\n", (unsigned char)-1);
    printf("%d\n", u > neg);
    printf("%d\n", (unsigned short)neg == 65533);
    printf("%d\n", ((char)200 < 0) + ((unsigned char)200 > 0));
    printf("%lu\n", sizeof(int) * (size_t)neg);
    printf("%d\n", u / 3 > 1000000000);
    printf("%ld\n", -u);
    printf("%d\n", !u + !big);
    return 0;
}
