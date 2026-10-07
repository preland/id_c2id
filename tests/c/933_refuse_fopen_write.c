/* refused: fopen with a write mode */
#include <stdio.h>
int main(void){ FILE *f = fopen("/tmp/c2id_repro_out.txt", "w"); if (!f) return 1; fputs("hi\n", f); fclose(f); return 0; }
