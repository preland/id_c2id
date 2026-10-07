// A heap that outgrows its first island: blocks bigger than an island, many
// small ones, frees in an order that exercises coalescing both ways, and a
// realloc that moves.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
    enum { N = 3000 };
    char* p[N];
    long check = 0;
    for (int i = 0; i < N; i++) {
        p[i] = malloc(100 + (i % 17) * 40);
        memset(p[i], i & 255, 100 + (i % 17) * 40);
    }
    for (int i = 0; i < N; i += 2) free(p[i]);
    for (int i = 0; i < N; i += 2) {
        p[i] = malloc(60 + (i % 13) * 50);
        memset(p[i], (i + 7) & 255, 60 + (i % 13) * 50);
    }
    for (int i = 0; i < N; i++) check += (unsigned char)p[i][50] * (i + 1);
    for (int i = N - 1; i >= 0; i -= 3) free(p[i]);
    char* big = malloc(3 * 1024 * 1024);
    memset(big, 9, 3 * 1024 * 1024);
    char* grown = realloc(big, 5 * 1024 * 1024);
    check += grown[0] + grown[3 * 1024 * 1024 - 1];
    char* zero = calloc(70000, 8);
    for (int i = 0; i < 70000 * 8; i++) check += zero[i];
    printf("%ld\n", check);
    for (int i = 0; i < N; i++) if (i % 3 != (N - 1) % 3) free(p[i]);
    free(grown);
    free(zero);
    printf("%d\n", malloc((size_t)1 << 40) == NULL);
    return 0;
}
