#include <stdio.h>
#include <unistd.h>

int main(void) {
  char buf[16];
  long n, total = 0;
  const char *hello = "hello via write\n";
  write(1, hello, 16);
  n = read(0, buf, 4);
  printf("read %ld [%c%c%c%c]\n", n, buf[0], buf[1], buf[2], buf[3]);
  while ((n = read(0, buf, sizeof buf)) > 0) total += n;
  printf("rest %ld\n", total);
  printf("write bad fd %ld\n", (long)write(77, hello, 1));
  printf("close %d\n", close(0) >= -1);
  return 0;
}
