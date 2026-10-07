#include <stdio.h>
#include <string.h>
static unsigned seed = 12345u;
static unsigned next(void) { seed = seed * 1103515245u + 12345u; return (seed >> 8) & 0xffff; }
static unsigned mix(unsigned h, int c) { return (h ^ (unsigned)(c & 255)) * 16777619u; }
int main(void) {
    char buf[600];
    unsigned h = 2166136261u;
    long total = 0;
    int ops = 0;
    for (;;) {
        unsigned op = next() % 5;
        if (op == 0) {
            size_t want = next() % 590 + 1;
            size_t got = fread(buf, 1, want, stdin);
            for (size_t i = 0; i < got; i++) h = mix(h, buf[i]);
            total += got;
            if (got < want) break;
        } else if (op == 1) {
            if (!fgets(buf, sizeof buf, stdin)) break;
            for (char *p = buf; *p; p++) h = mix(h, *p);
            total += strlen(buf);
        } else if (op == 2) {
            int n = next() % 50 + 1;
            for (int i = 0; i < n; i++) {
                int c = getchar();
                if (c == EOF) { printf("eof in getchar after %ld\n", total); goto done; }
                h = mix(h, c);
                total++;
            }
        } else if (op == 3) {
            int c = getchar();
            if (c == EOF) break;
            ungetc(c, stdin);
        } else {
            size_t got = fread(buf, 7, next() % 80 + 1, stdin);
            for (size_t i = 0; i < got * 7; i++) h = mix(h, buf[i]);
            total += got * 7;
        }
        ops++;
    }
done:
    printf("%d ops %ld bytes hash %u\n", ops, total, h);
    return 0;
}
