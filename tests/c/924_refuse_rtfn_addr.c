#include <stdio.h>

typedef struct {
  int (*write)(const char *fmt, ...);
  int (*puts)(const char *s);
} hooks;

static hooks h = {printf, puts};

int main(void) {
  h.write("%d\n", 1);
  h.puts("x");
  return 0;
}
