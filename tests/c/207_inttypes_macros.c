#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
int main(void) {
  int64_t a = -5000000000LL; uint64_t b = 18446744073709551615ULL; uint32_t c = 4000000000u; int32_t d = -7;
  printf("%" PRId64 " %" PRIu64 " %" PRIx64 " %" PRIu32 " %" PRId32 " %" PRIX32 "\n", a, b, b, c, d, c);
  printf("%d %d\n", (int)imaxabs(-9), (int)sizeof(intmax_t));
  printf("%s|%s|%s\n", strerror(2), strerror(13), strerror(0));
  printf("%d %d %d\n", (int)strspn("aabbcc", "ab"), (int)strcspn("aabbcc", "c"), strpbrk("hello", "lo") - "hello");
  printf("%.3f\n", (double)strtof("2.5", 0));
  printf("%d %d\n", (int)INT64_C(7), (int)(UINT64_C(3) + INT8_C(1)));
  return 0;
}
