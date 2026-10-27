int printf(const char *f, ...);
struct S { long x; char c; };
#define MIN(a, b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a < _b ? _a : _b; })
#define MAX(a, b) ({ __auto_type _a = (a); __auto_type _b = (b); _a > _b ? _a : _b; })
int arr[4] = {1, 2, 3, 4};
int main(void) {
  int a = 3, b = 9;
  long l = 100000000000L;
  struct S s = {5, 'x'};
  typeof(a) c = a + 1;
  typeof(l) d = l * 2;
  typeof(s) t = s;
  typeof(&a) pa = &a;
  typeof(arr) copy;
  typeof(a) *pb = &b;
  __auto_type e = l + a;
  __auto_type p = arr;
  __typeof__(s.x) f = s.x + 1;
  typeof(int) g = 4;
  copy[0] = 7;
  printf("%d %ld %ld %d %d\n", c, d, t.x, *pa, copy[0]);
  printf("%d %ld %d %ld %d %d\n", *pb, e, p[2], f, g, (int)sizeof(copy));
  printf("%d %ld %ld\n", MIN(a, b), MAX(l, 5L), (long)MAX(a, b));
  return 0;
}
