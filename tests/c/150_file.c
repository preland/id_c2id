#include <stdio.h>
#include <string.h>

int main(void) {
  FILE *f = fopen(__FILE__, "r");
  char line[80], blob[16];
  int n = 0, first, got;
  long count = 0;
  const char *none = "/nonexistent/c2id/file";
  if (!f) {
    printf("no file\n");
    return 1;
  }
  fgets(line, sizeof line, f);
  printf("first line [%s]", line);
  first = fgetc(f);
  ungetc(first, f);
  got = (int)fread(blob, 1, 8, f);
  blob[8] = 0;
  printf("%d %c %d [%s]\n", got, first, first == blob[0], blob);
  while (fgets(line, sizeof line, f)) n++;
  printf("more lines %d\n", n > 5);
  printf("eof %d\n", fgetc(f));
  fclose(f);
  printf("missing %d\n", fopen(none, "r") == 0);
  f = fopen(__FILE__, "rb");
  while (fgetc(f) != EOF) count++;
  printf("bytes %d\n", count > 200);
  fclose(f);
  return 0;
}
