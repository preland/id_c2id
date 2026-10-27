#include <stdio.h>
#include <string.h>

int main(void) {
  const char *msg = "to stderr\n";
  fputs("one", stderr);
  fputs(" two", stderr);
  fputc('\n', stderr);
  fprintf(stderr, "three %d\n", 3);
  fwrite(msg, 1, strlen(msg), stderr);
  fflush(stderr);
  return 3;
}
