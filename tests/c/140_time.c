#include <stdio.h>
#include <time.h>

int main(void) {
  time_t t = time(0), u = 0;
  clock_t c = clock();
  time(&u);
  printf("%d %d\n", t > 1700000000, u >= t);
  printf("%d\n", c >= 0 && c < 60 * CLOCKS_PER_SEC);
  return 0;
}
