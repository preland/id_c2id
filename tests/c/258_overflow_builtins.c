#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#define T(op, a, b, ty) do { ty r = 0; int o = __builtin_##op##_overflow((a), (b), &r); printf("%s %d %lld\n", #op #ty, o, (long long)r); } while (0)
int main(void) {
  T(add, INT_MAX, 1, int);
  T(add, INT_MAX, 0, int);
  T(sub, INT_MIN, 1, int);
  T(mul, 65536, 65536, int);
  T(mul, 46340, 46340, int);
  T(add, 200, 100, unsigned char);
  T(add, 100, 100, signed char);
  T(sub, 0u, 1u, unsigned);
  T(mul, 0xFFFFFFFFu, 0xFFFFFFFFu, unsigned);
  T(add, LLONG_MAX, 1LL, long long);
  T(sub, LLONG_MIN, 1LL, long long);
  T(mul, LLONG_MIN, -1LL, long long);
  T(mul, 3037000500LL, 3037000500LL, long long);
  T(mul, 3037000499LL, 3037000499LL, long long);
  T(add, ULLONG_MAX, 1ULL, unsigned long long);
  T(sub, 0ULL, 1ULL, unsigned long long);
  T(mul, ULLONG_MAX, 2ULL, unsigned long long);
  T(mul, 4294967296ULL, 4294967296ULL, unsigned long long);
  T(add, -1, 1u, unsigned);
  T(add, -5, 3, short);
  T(add, 70000, 0, short);
  T(sub, -1, -1, unsigned long);
  T(add, ULLONG_MAX, -1LL, long long);
  T(mul, -3, 5, signed char);
  unsigned long ul; int sz = __builtin_uaddl_overflow(~0ul, 1ul, &ul);
  int sa; int s2 = __builtin_sadd_overflow(2000000000, 2000000000, &sa);
  unsigned long long u3; int s3 = __builtin_umulll_overflow(1ULL << 40, 1ULL << 30, &u3);
  printf("%d %lu %d %d %d %llu\n", sz, ul, s2, sa, s3, u3);
  return 0;
}
