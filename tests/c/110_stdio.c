#include <stdio.h>
#include <string.h>

int main(void) {
  const char *msg = "bytes\n";
  int i;
  fputs("fputs to stdout\n", stdout);
  fputc('A', stdout);
  fputc('\n', stdout);
  putchar('B');
  putchar('\n');
  fprintf(stdout, "fprintf %d %s\n", 7, "ok");
  fwrite(msg, 1, strlen(msg), stdout);
  fwrite("xyz", 1, 3, stdout);
  fputc('\n', stdout);
  for (i = 0; i < 3; i++) putc('0' + i, stdout);
  fputc('\n', stdout);
  fflush(stdout);
  return 0;
}
