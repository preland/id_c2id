#include <stdio.h>
int main(void) {
  FILE *f = fopen("/proc/self/exe", "rb");
  long sz;
  int a, b, c;
  if (!f) return 1;
  a = fgetc(f); b = fgetc(f); c = fgetc(f);
  printf("%d %c%c %ld\n", a == 127, b, c, ftell(f));
  fseek(f, 0, SEEK_END); sz = ftell(f);
  printf("%d\n", sz > 1000);
  rewind(f);
  a = fgetc(f);
  printf("%ld %d\n", ftell(f), a == 127);
  fseek(f, 1, SEEK_SET);
  printf("%c\n", fgetc(f));
  fseek(f, -2, SEEK_CUR);
  printf("%ld\n", ftell(f));
  fclose(f);
  return 0;
}
