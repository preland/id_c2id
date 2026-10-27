#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
    printf("%d\n", argc);
    for (int i = 1; i < argc; i++) printf("%d [%s] %d\n", i, argv[i], (int)strlen(argv[i]));
    printf("%d\n", argv[argc] == 0);
    return argc > 2 ? 3 : 0;
}
