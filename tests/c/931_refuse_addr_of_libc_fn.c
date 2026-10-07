/* refused: address of a runtime-provided libc function (strlen) stored in a table */
#include <stdio.h>
#include <string.h>
typedef struct { const char *name; size_t (*fn)(const char *); } Ent;
static const Ent tab[] = { { "strlen", strlen } };
int main(void){ printf("%d\n", (int)tab[0].fn("hello")); return 0; }
