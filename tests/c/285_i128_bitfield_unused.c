#include <stdio.h>
struct Unused { unsigned __int128 flags : 70; int tag; };
static inline int unused_reader(struct Unused *u) { return (int)u->flags; }
int main(void) { printf("%d\n", 7); return 0; }
