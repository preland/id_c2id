// Function pointers and indirect dispatch -- the struct-of-ops kernel idiom.
#include <stdio.h>
static int add(int a, int b) { return a + b; }
static int mul(int a, int b) { return a * b; }
static int sub(int a, int b) { return a - b; }
struct ops { const char *name; int (*fn)(int, int); };
int main(void) {
    struct ops table[3] = {{"add", add}, {"mul", mul}, {"sub", sub}};
    for (int i = 0; i < 3; i++)
        printf("%s=%d ", table[i].name, table[i].fn(7, 3));
    printf("\n");
    int (*f)(int, int) = table[1].fn;
    printf("%d\n", f(6, 6));
    return 0;
}
