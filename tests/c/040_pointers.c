// Pointers, address-of, pointer arithmetic, arrays.
#include <stdio.h>
static int sum(const int *p, int n) {
    int t = 0;
    for (const int *q = p; q < p + n; q++) t += *q;
    return t;
}
int main(void) {
    int a[6] = {1, 2, 3, 4, 5, 6};
    int x = 42, *px = &x;
    *px += 1;
    printf("%d %d\n", x, *px);
    printf("%d\n", sum(a, 6));
    printf("%d %d %d\n", a[2], *(a + 2), *(2 + a));
    int *p = a + 4;
    printf("%ld %d\n", (long)(p - a), p[-1]);
    char buf[8];
    for (int i = 0; i < 7; i++) buf[i] = 'a' + i;
    buf[7] = 0;
    printf("%s\n", buf);
    return 0;
}
