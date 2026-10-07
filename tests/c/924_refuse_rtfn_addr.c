#include <stdio.h>
#include <stdlib.h>

typedef struct {
  void *(*allocate)(size_t size);
  void (*deallocate)(void *p);
} hooks;

static hooks h = {malloc, free};

int main(void) {
  char *p = h.allocate(8);
  printf("%d\n", p != 0);
  h.deallocate(p);
  return 0;
}
