#include <stdio.h>
#include <stdlib.h>

static void bye(void) {
  printf("bye\n");
  exit(3);
}

static void fine(void) { printf("fine\n"); }

struct ops { void (*run)(void); };

static void call_it(void (*f)(void)) {
  f();
  printf("after call_it\n");
}

int main(void) {
  struct ops o;
  void (*table[2])(void);
  table[0] = fine;
  table[1] = bye;
  o.run = fine;
  o.run();
  printf("one\n");
  call_it(table[0]);
  table[1]();
  printf("after table\n");
  call_it(table[1]);
  printf("unreachable\n");
  return 0;
}
