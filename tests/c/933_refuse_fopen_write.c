/* refused: fopen with a mode that is not r, w or a followed by b and + */
#include <stdio.h>
int main(void){ FILE *f = fopen("/tmp/c2id_repro_out.txt", "w+e"); if (!f) return 1; fputs("hi\n", f); fclose(f); return 0; }
