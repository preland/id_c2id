#include <stdio.h>
int runtime_len(int);
struct Big { char head; char tail[(__builtin_constant_p(4096ULL) ? 12 : runtime_len(3))]; };
int main(void) { printf("%d\n", (int)sizeof(struct Big)); return 0; }
