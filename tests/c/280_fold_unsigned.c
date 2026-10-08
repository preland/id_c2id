#include <stdio.h>
enum {
  U1 = (unsigned char)300,
  U2 = (short)70000,
  U3 = (char)200,
  U4 = (unsigned)-1 > 0,
  U5 = -1 < 0u,
  U6 = (unsigned char)-1 >> 1,
  U7 = ~0u == 0xffffffff,
  U8 = (int)0xffffffffu,
  U9 = -1 < 0,
  V1 = 0x7fffffffffffffffLL / 3 > 0x7fffffff,
  V2 = ~0UL >> 60,
  V3 = (1ULL << 63) >> 62,
  V4 = 18446744073709551615ULL / 3 == 6148914691236517205ULL,
  V5 = 18446744073709551615ULL % 10,
  V6 = -7 / 2,
  V7 = -7 % 3,
  V8 = (unsigned)(-7) % 10u,
  V9 = 4294967295u + 1u,
  W1 = 2147483647u * 2u,
  W2 = 0u - 1 > 0,
  W3 = (long)-1 < (unsigned long)0,
  W4 = (int)(1ULL << 32 >> 1 >> 31),
  W5 = sizeof(int) - 5 > 0,
  W6 = -(unsigned char)1,
  W7 = (0xffffffffffffffffULL > 0x7fffffffffffffffLL),
  W8 = (-9223372036854775807LL - 1) / -1 == (-9223372036854775807LL - 1),
  W9 = 1 ? -1 : 0u
};
int main(void) {
  printf("%d %d %d %d %d %d %d %d %d\n", U1, U2, U3, U4, U5, U6, U7, U8, U9);
  printf("%d %d %d %d %d %d %d %d %d\n", V1, V2, V3, V4, V5, V6, V7, V8, (int)V9);
  printf("%d %d %d %d %d %d %d %d\n", (int)W1, W2, W3, W4, W5, W6, W7, W8);
  printf("%d\n", (int)W9);
  return 0;
}
