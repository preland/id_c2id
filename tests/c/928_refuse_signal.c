#include <stdio.h>
#include <signal.h>
static void on_int(int s) { printf("got %d\n", s); }
int main(void) {
  signal(SIGINT, on_int);
  raise(SIGINT);
  return 0;
}
