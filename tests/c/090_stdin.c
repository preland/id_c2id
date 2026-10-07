#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
  char line[40], word[16];
  char blob[8];
  int c, n, a, b, got, lines;
  long total = 0;
  c = getchar();
  printf("first %d\n", c);
  ungetc(c, stdin);
  c = fgetc(stdin);
  printf("again %d\n", c);
  ungetc('Z', stdin);
  c = fgetc(stdin);
  printf("pushed %d then %d\n", c, fgetc(stdin));
  if (fgets(line, sizeof line, stdin)) printf("rest [%s]", line);
  if (fgets(line, 6, stdin)) printf("short [%s]\n", line);
  if (fgets(line, sizeof line, stdin)) printf("tail [%s]", line);
  got = (int)fread(blob, 1, 8, stdin);
  printf("fread %d %d %d\n", got, blob[0], blob[7]);
  got = (int)fread(blob, 1, 1, stdin);
  printf("one %d %d\n", got, blob[0]);
  got = (int)fread(blob, 4, 2, stdin);
  printf("pairs %d\n", got);
  if (fgets(line, sizeof line, stdin) && sscanf(line, "%d %d %s", &a, &b, word) == 3) printf("scanned %d %d %s\n", a, b, word);
  n = 0;
  while (n < 5 && (c = getchar()) != EOF) { total += c; n++; }
  printf("five bytes, sum %ld\n", total);
  lines = 0;
  while (fgets(line, sizeof line, stdin)) { lines++; printf("line %d [%s]%s", lines, line, strchr(line, '\n') ? "" : "\n"); }
  c = fgetc(stdin);
  printf("eof %d %d\n", c, getchar());
  return 0;
}
