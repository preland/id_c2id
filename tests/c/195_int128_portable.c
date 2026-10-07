#include <stdio.h>
typedef struct { unsigned long long low64, high64; } U128;
static U128 mul64to128(unsigned long long a, unsigned long long b) {
#ifdef __SIZEOF_INT128__
  __uint128_t const product = (__uint128_t)a * (__uint128_t)b;
  U128 r = { (unsigned long long)product, (unsigned long long)(product >> 64) };
  return r;
#else
  unsigned long long lo_lo = (a & 0xFFFFFFFF) * (b & 0xFFFFFFFF);
  unsigned long long hi_lo = (a >> 32) * (b & 0xFFFFFFFF);
  unsigned long long lo_hi = (a & 0xFFFFFFFF) * (b >> 32);
  unsigned long long hi_hi = (a >> 32) * (b >> 32);
  unsigned long long cross = (lo_lo >> 32) + (hi_lo & 0xFFFFFFFF) + lo_hi;
  U128 r;
  r.high64 = (hi_lo >> 32) + (cross >> 32) + hi_hi;
  r.low64 = (cross << 32) | (lo_lo & 0xFFFFFFFF);
  return r;
#endif
}
int main(void) {
  U128 r = mul64to128(0x9E3779B185EBCA87ULL, 0xC2B2AE3D27D4EB4FULL);
  printf("%016llx %016llx\n", r.high64, r.low64);
  return 0;
}
