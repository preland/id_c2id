#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(void) {
  char buf[32], out[32];
  char *p, *q;
  int i, *v;
  unsigned long bits = 0xf0UL;
  sprintf(buf, "%d-%s", 12, "ab");
  snprintf(out, 5, "%s", "truncated");
  puts(buf);
  puts(out);
  printf("%d %d %d %d %d\n", !!isdigit('7'), !!isupper('a'), !!islower('a'), !!isalpha('9'), !!isalnum('_'));
  printf("%d %d %d %c %c\n", !!isspace('\t'), !!isxdigit('F'), !!isxdigit('g'), toupper('q'), tolower('Q'));
  printf("%d %d %d\n", memcmp("abc", "abd", 3) < 0, strcmp("b", "a") > 0, strncmp("abcx", "abcy", 3));
  memcpy(buf, "0123456789", 11);
  memmove(buf + 2, buf, 6);
  printf("%s\n", buf);
  p = (char *)calloc(4, 4);
  q = (char *)realloc(p, 64);
  v = (int *)malloc(3 * sizeof(int));
  for (i = 0; i < 3; i++) v[i] = i * 7;
  printf("%d %d %d %d\n", q != 0, v[2], (int)strlen("four"), (int)q[3]);
  free(v);
  free(q);
  printf("%d %d %d %d\n", __builtin_ctz(bits), __builtin_clz(1u), __builtin_ctzll(bits << 20), __builtin_clzll(1ULL));
  printf("%lx %x\n", (unsigned long)__builtin_bswap64(0x0102030405060708UL), __builtin_bswap32(0x01020304u));
  exit(5);
}
