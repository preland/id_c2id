/* refused: getenv and atexit not provided by the runtime */
#include <stdio.h>
#include <stdlib.h>
static void bye(void) { puts("bye"); }
int main(void){ const char *e = getenv("HOME"); atexit(bye); printf("%d\n", e != 0); return 0; }
