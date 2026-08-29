// for / while / do-while / switch / break / continue / goto.
#include <stdio.h>
int main(void) {
    int total = 0;
    for (int i = 0; i < 10; i++) {
        if (i == 3) continue;
        if (i == 8) break;
        total += i;
    }
    printf("%d\n", total);
    int n = 5, f = 1;
    do { f *= n; n--; } while (n > 1);
    printf("%d\n", f);
    for (int k = 0; k < 5; k++) {
        switch (k) {
        case 0: printf("zero "); break;
        case 1:
        case 2: printf("small "); break;
        default: printf("big "); break;
        }
    }
    printf("\n");
    int i = 0;
again:
    i++;
    if (i < 4) goto again;
    printf("%d\n", i);
    return 0;
}
