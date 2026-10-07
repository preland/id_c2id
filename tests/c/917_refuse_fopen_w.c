#include <stdio.h>
int main(void) {
  FILE *f = fopen("/tmp/c2id_refuse_probe.txt", "w");
  if (!f) return 1;
  fprintf(f, "hi\n");
  fclose(f);
  return 0;
}
