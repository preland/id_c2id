int printf(const char *fmt, ...);
struct A { struct { char d[__alignof__(void *)]; } b; };
struct B { char c; double d; short s; };
int main(void) {
  int x = 0;
  char buf[_Alignof(struct B) * 2];
  printf("%d %d %d\n", (int)sizeof(struct A), (int)__alignof__(struct B), (int)sizeof(buf));
  printf("%d %d %d\n", (int)_Alignof(char), (int)__alignof(short), (int)__alignof__(x));
  printf("%d\n", (int)(__alignof__(long) + sizeof(void *)));
  return 0;
}
