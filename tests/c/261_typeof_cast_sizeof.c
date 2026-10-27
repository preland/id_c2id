int printf(const char *f, ...);
#define READ_ONCE(x) (*(const volatile __typeof_unqual__(x) *)&(x))
#define MAXV(a, b) ((__typeof__(a))((a) > (b) ? (a) : (b)))
struct S { long v; char c; };
unsigned long g = 12;
int main(void) {
  int a = 5; long l = 70000000000L; unsigned char uc = 200;
  struct S s = { 9, 'q' };
  printf("%d %ld %d\n", READ_ONCE(a), READ_ONCE(l), READ_ONCE(uc));
  printf("%ld %d\n", READ_ONCE(s.v), (int)sizeof(typeof(s)));
  printf("%d %d %d\n", (int)sizeof(__typeof__(l)), MAXV(a, 9), (int)MAXV(uc, 100));
  printf("%lu\n", (unsigned long)(typeof(g))-1);
  return 0;
}
