/* SILENT MISCOMPILE: __uint128_t multiply; prints zeros, gcc prints 7854787aa57880a8 def35b010f796ca9 */
#include <stdio.h>
typedef struct { unsigned long long low64, high64; } U128;
static U128 mul64to128(unsigned long long a, unsigned long long b) {
  __uint128_t const product = (__uint128_t)a * (__uint128_t)b;
  U128 r;
  r.low64 = (unsigned long long)product;
  r.high64 = (unsigned long long)(product >> 64);
  return r;
}
int main(void){
  U128 r = mul64to128(0x9E3779B185EBCA87ULL, 0xC2B2AE3D27D4EB4FULL);
  printf("%016llx %016llx\n", r.high64, r.low64);
  return 0;}
