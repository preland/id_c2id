#include <stdio.h>
#include <stdint.h>
struct S { int a; };
typedef int myint;
#define IS_INT(x) __builtin_types_compatible_p(__typeof__(x), int)
int pick(int v) {
  return __builtin_choose_expr(sizeof(long) == 8, v * 2, (void)0);
}
int main(void) {
  unsigned x = 0xF0F0u; unsigned long long y = 0x8000000000000001ULL; unsigned long z = 40;
  printf("%d %d %d %d\n", __builtin_popcount(x), __builtin_popcountl(z), __builtin_popcountll(y), __builtin_parity(7u));
  printf("%d %d %d %d %d\n", __builtin_ffs(0), __builtin_ffs(8), __builtin_ffsll(1ULL << 40), __builtin_ctzl(z), __builtin_clzl(z));
  printf("%x %x\n", __builtin_bswap16(0x1234), (unsigned)__builtin_bswap32(0x11223344u));
  printf("%d %d %d\n", __builtin_constant_p(x) >= 0, __builtin_constant_p(5) >= 0, pick(21));
  printf("%d %d %d %d\n", __builtin_types_compatible_p(int, myint), __builtin_types_compatible_p(int, long), __builtin_types_compatible_p(struct S, struct S), __builtin_types_compatible_p(int *, int *));
  printf("%d %d\n", __builtin_types_compatible_p(char[3], char[3]), __builtin_types_compatible_p(unsigned, int));
  int v = __builtin_choose_expr(1, 7, 8) + __builtin_choose_expr(0, 7, 8);
  printf("%d\n", v);
  if (x == 0) __builtin_unreachable();
  return 0;
}
