#include <stdio.h>

int main(void) {
  int a = -1, b = -1, c = -1, n = -1, r;
  unsigned u = 0;
  long l = 0;
  short sh = 0;
  char s[20] = "", t[20] = "", ch = 0;
  signed char sc = 0;
  r = sscanf("12 -7 0x1f", "%d %d %i", &a, &b, &c);
  printf("%d: %d %d %d\n", r, a, b, c);
  r = sscanf("abc def", "%s %s", s, t);
  printf("%d: %s %s\n", r, s, t);
  r = sscanf("  42abc", "%d%s", &a, s);
  printf("%d: %d %s\n", r, a, s);
  r = sscanf("ff 777 123456789012", "%x %o %ld", &u, &b, &l);
  printf("%d: %u %d %ld\n", r, u, b, l);
  r = sscanf("key=val", "key=%s", s);
  printf("%d: %s\n", r, s);
  r = sscanf("x 300", "%c %hd", &ch, &sh);
  printf("%d: %c %d\n", r, ch, sh);
  r = sscanf("-5 77", "%hhd %hhd", &sc, &ch);
  printf("%d: %d %d\n", r, sc, ch);
  r = sscanf("xy", "%c%c", &ch, &s[0]);
  printf("%d: %c %c\n", r, ch, s[0]);
  r = sscanf("10 20", "%d %*d %d", &a, &b);
  printf("%d: %d %d\n", r, a, b);
  r = sscanf("7 rest", "%d%n", &a, &n);
  printf("%d: %d %d\n", r, a, n);
  r = sscanf("100%", "%d%%", &a);
  printf("%d: %d\n", r, a);
  b = -1;
  r = sscanf("5x7", "%d%%%d", &a, &b);
  printf("%d: %d %d\n", r, a, b);
  r = sscanf("nope", "%d", &a);
  printf("%d\n", r);
  r = sscanf("", "%d", &a);
  printf("%d\n", r);
  r = sscanf("5 x", "%d %d", &a, &b);
  printf("%d: %d\n", r, a);
  return 0;
}
