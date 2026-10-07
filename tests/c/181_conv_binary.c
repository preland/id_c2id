#include <stdio.h>
#include <stddef.h>
static long mulw(int a, long b) { return a * b; }
static long mulnarrow(int a, int b) { return (long)(a * b); }
static unsigned long umix(unsigned a, long b) { return a + b; }
static int lt(int a, unsigned b) { return a < b; }
static int ltl(long a, unsigned b) { return a < b; }
static int ltul(long a, unsigned long b) { return a < b; }
static long shl(int a, long n) { return a << n; }
static long shl2(long a, char n) { return a << n; }
static unsigned shru(unsigned a, long n) { return a >> n; }
static int shrs(int a, unsigned n) { return a >> n; }
static int chadd(char a, char b) { return a + b; }
static int shadd(short a, short b) { return a * b; }
static int uchadd(unsigned char a, unsigned char b) { return a - b; }
static unsigned ushsub(unsigned short a, unsigned short b) { return a - b; }
static long cmpd(int a, unsigned b) { return (a - b) > 0; }
static int divs(int a, unsigned b) { return a / b; }
static long divl(long a, unsigned b) { return a / b; }
static int mods(int a, unsigned b) { return a % b; }
static long negu(unsigned a) { return -a; }
static long notu(unsigned a) { return ~a; }
static long notc(unsigned char a) { return ~a; }
static long bitmix(int a, unsigned long b) { return a & b; }
static long bitmix2(int a, unsigned b) { return a | b; }
int main(void) {
    int big = 2000000000;
    printf("%ld\n", mulw(big, 3));
    printf("%ld\n", mulnarrow(big, 3));
    printf("%lu\n", umix(1u, -2L));
    printf("%lu\n", umix(4000000000u, 1L));
    printf("%d %d\n", lt(-1, 1u), lt(1, 2u));
    printf("%d %d\n", ltl(-1, 1u), ltl(5000000000L, 1u));
    printf("%d %d\n", ltul(-1, 1ul), ltul(1, 2ul));
    printf("%ld\n", shl(1, 40));
    printf("%ld\n", shl(big, 3));
    printf("%ld\n", shl2(1, 40));
    printf("%u\n", shru(0x80000000u, 31));
    printf("%d\n", shrs(-16, 2u));
    printf("%d\n", chadd(100, 100));
    printf("%d\n", chadd(-100, -100));
    printf("%d\n", shadd(30000, 30000));
    printf("%d\n", uchadd(3, 5));
    printf("%u\n", ushsub(3, 5));
    printf("%ld\n", cmpd(3, 5u));
    printf("%d\n", divs(-6, 4u));
    printf("%ld\n", divl(-6L, 4u));
    printf("%d\n", mods(-7, 4u));
    printf("%ld\n", negu(5u));
    printf("%ld\n", notu(5u));
    printf("%ld\n", notc(5));
    printf("%ld\n", bitmix(-1, 255ul));
    printf("%ld\n", bitmix2(-1, 255u));
    printf("%d\n", (int)sizeof(int) - 8);
    printf("%d\n", sizeof(long) - 9 > 0);
    printf("%lu\n", sizeof(int) - 8);
    printf("%d\n", -1 < sizeof(int));
    printf("%lu\n", sizeof(char) * 3);
    printf("%d\n", sizeof(short[3]) == 6);
    printf("%d %d\n", lt(-5, 4294967295u), lt(-1, 0u));
    printf("%d\n", ltl(-5, 4294967295u));
    return 0;
}
