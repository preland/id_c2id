int printf(const char *fmt, ...);
int main(void) {
  char buf[8] = {10, 11, 12, 13, 14, 15, 16, 17};
  void *p = buf;
  void *q = p + 3;
  void *r = q;
  r += 2;
  r++;
  printf("%d %d %d\n", *(char *)q, *(char *)r, (int)((char *)(p + 5) - (char *)p));
  printf("%d %d\n", (int)((char *)r - (char *)p), (int)sizeof(void));
  printf("%d\n", (int)(r - p));
  return 0;
}
