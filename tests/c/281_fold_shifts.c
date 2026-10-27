#include <stdio.h>
enum {
  S1 = (int)(1ULL << 40 >> 38),
  S2 = 1 << 30,
  S3 = (int)((1U << 31) >> 31),
  S4 = (int)(((unsigned long long)1 << 63) >> 63),
  S5 = (int)(0x8000000000000000ULL >> 60),
  S6 = (int)(0x7000000000000000LL >> 60),
  S7 = (int)((-1LL) >> 1 == -1),
  S8 = (int)(~0ULL >> 1 == 0x7fffffffffffffffULL),
  S9 = (int)(0xdeadbeefcafebabeULL >> 32 & 0xff)
};
char arr1[(1ULL << 33) >> 31];
char arr2[sizeof(long) * 2 - 1u];
#define HUGE (0x100000000ULL * 4)
int main(void) {
  printf("%d %d %d %d %d %d %d %d %d\n", S1, S2, S3, S4, S5, S6, S7, S8, S9);
  printf("%d %d\n", (int)sizeof(arr1), (int)sizeof(arr2));
  switch (3) {
    case (unsigned char)259 ... (unsigned char)261: printf("in range\n"); break;
    default: printf("out of range\n"); break;
  }
  printf("%d\n", __builtin_choose_expr(HUGE, 11, 22));
  printf("%d\n", __builtin_choose_expr(HUGE >> 34, 33, 44));
  return 0;
}
