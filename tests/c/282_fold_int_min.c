#include <stdio.h>
enum { A7 = 1 << 31, A8 = (int)(1u << 31), A0 = -2147483647 - 1, A3 = 5 };
int main(void) { printf("%lld %lld %lld %d\n", (long long)A7, (long long)A8, (long long)A0, A3); return 0; }
