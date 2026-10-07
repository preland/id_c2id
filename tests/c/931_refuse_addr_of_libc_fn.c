/* refused: address of a runtime-provided libc function with no wrapper (sprintf) stored in a table */
#include <stdio.h>
typedef struct { const char *name; int (*fn)(char *, const char *, ...); } Ent;
static const Ent tab[] = { { "sprintf", sprintf } };
int main(void){ char b[16]; tab[0].fn(b, "%d", 5); printf("%s\n", b); return 0; }
