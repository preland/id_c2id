/* The same calls as tests/crt/prog, in C.
 *
 * crt is C's semantics written in id, so the only test that means anything is
 * the one that asks C. This program's output must be byte-identical to the id
 * program's, minus the two allocator lines -- see tests/crt/run.sh. */
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("d=%d u=%u x=%x X=%X o=%o\n", -42, (unsigned)-1, 48879, 48879, 511);
    printf("[%5d][%-5d][%05d][%5s][%c]\n", 42, 42, 42, "hi", 65);
    printf("hh=%hhd h=%hd l=%ld pct=%%\n", 300, 70000, -1L);
    printf("len %zu\n", strlen("hello"));
    printf("cmp %d\n", strcmp("hello", "hellp"));
    {
        char buf[16];
        memset(buf, 65, 4);
        printf("memset %d\n", (int)(unsigned char)buf[3]);
    }
    return 0;
}
