#include <stdarg.h>
#include <stdio.h>
static long f(int n, ...) { va_list ap; va_start(ap, n); __int128 x = va_arg(ap, __int128); va_end(ap); return (long)(x >> 3); }
int main(void) { __int128 big = 80; printf("%ld\n", f(1, big)); return 0; }
