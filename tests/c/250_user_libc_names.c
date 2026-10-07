typedef unsigned long size_t;
int printf(const char *f, ...);
size_t strlen(const char *s) { size_t n = 0; while (s[n]) n++; return n; }
int memcmp(const void *a, const void *b, size_t n) { const unsigned char *x = a, *y = b; while (n--) { if (*x != *y) return *x - *y; x++; y++; } return 0; }
int strcmp(const char *a, const char *b) { while (*a && *a == *b) { a++; b++; } return *(const unsigned char *)a - *(const unsigned char *)b; }
int format(int x) { return x * 3; }
int (*fp)(const void *, const void *, size_t) = memcmp;
int main(void) {
  printf("%d %d %d %d\n", (int)strlen("abc"), memcmp("ab", "ab", 2), format(4), fp("a", "b", 1));
  printf("%d\n", strcmp("abd", "abc") > 0);
  return 0;
}
