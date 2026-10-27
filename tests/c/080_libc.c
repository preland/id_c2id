#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
static int cmpint(const void *a, const void *b) { int x = *(const int *)a, y = *(const int *)b; return (x > y) - (x < y); }
int main(void) {
  int v[8] = {5, 3, 9, 1, 7, 2, 8, 4};
  int i, key = 7, *hit;
  char buf[64], *e;
  long n;
  printf("%d %ld %d\n", atoi("  -42xyz"), atol("123456789012"), atoi("abc"));
  n = strtol("0x1fz", &e, 16);
  printf("%ld %s\n", n, e);
  n = strtol("  077", &e, 0);
  printf("%ld %d\n", n, (int)(e - "  077"));
  n = strtol("zzz", &e, 10);
  printf("%ld %d\n", n, *e == 'z');
  printf("%lu\n", strtoul("4000000000", 0, 10));
  qsort(v, 8, sizeof(int), cmpint);
  for (i = 0; i < 8; i++) printf("%d ", v[i]);
  printf("\n");
  hit = (int *)bsearch(&key, v, 8, sizeof(int), cmpint);
  printf("%d %d\n", hit != 0, hit ? (int)(hit - v) : -1);
  memset(buf, 'x', sizeof buf);
  strcpy(buf, "abc");
  strcat(buf, "def");
  strncat(buf, "ghijk", 2);
  printf("%s %d\n", buf, (int)strlen(buf));
  printf("%s %s %s\n", strrchr("a/b/c", '/'), strstr("hello world", "o w"), strchr("hello", 'l'));
  printf("%d\n", strstr("abc", "zz") == 0);
  printf("%s|%s|%d\n", strstr("hello world", "world"), strstr("hello", ""), (int)(strrchr(buf, 0) - buf));
  strncpy(buf, "xy", 5);
  printf("%d %d %d\n", buf[0], buf[1], buf[4]);
  e = strdup("dup");
  printf("%s %d\n", e, memchr("abcdef", 'd', 6) != 0);
  printf("%d %d %d %d\n", !!isprint(97), !!ispunct(33), !!iscntrl(10), !!isgraph(32));
  printf("%d %d %d\n", !!ispunct(97), !!ispunct(55), !!ispunct(32));
  printf("%d %d\n", abs(-5), (int)labs(-7));
  return 0;
}
