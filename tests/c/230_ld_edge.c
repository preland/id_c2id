#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>

typedef unsigned long long u64;

static long double mk(u64 m, unsigned se) {
  unsigned char b[16];
  long double x;
  memset(b, 0, 16);
  memcpy(b, &m, 8);
  b[8] = (unsigned char)(se & 255);
  b[9] = (unsigned char)(se >> 8);
  memcpy(&x, b, 16);
  return x;
}

static void hex(long double x) {
  unsigned char b[16];
  memcpy(b, &x, 16);
  for (int i = 9; i >= 0; i--) printf("%02x", b[i]);
}

static long double vals[12];
static const char *names[12] = {"+0", "-0", "dmin", "-dmin", "nmin", "one", "-one", "max", "inf", "-inf", "qnan", "snan"};

static void setup(void) {
  vals[0] = mk(0, 0);
  vals[1] = mk(0, 0x8000);
  vals[2] = mk(1, 0);
  vals[3] = mk(1, 0x8000);
  vals[4] = mk(0x8000000000000000ULL, 1);
  vals[5] = mk(0x8000000000000000ULL, 16383);
  vals[6] = mk(0x8000000000000000ULL, 0xBFFF);
  vals[7] = mk(0xFFFFFFFFFFFFFFFFULL, 32766);
  vals[8] = mk(0x8000000000000000ULL, 32767);
  vals[9] = mk(0x8000000000000000ULL, 0xFFFF);
  vals[10] = mk(0xC000000000000000ULL, 32767);
  vals[11] = mk(0xA000000000000001ULL, 32767);
}

int main(void) {
  setup();
  for (int i = 0; i < 12; i++) {
    printf("%-5s ", names[i]);
    hex(vals[i]);
    printf(" %d", vals[i] != vals[i]);
    if (vals[i] == vals[i] && vals[i] - vals[i] == 0) printf(" %.6Lg", vals[i]);
    printf("\n");
  }
  for (int i = 0; i < 12; i++) {
    for (int j = 0; j < 12; j++) {
      volatile long double a = vals[i], b = vals[j];
      printf("%d %d: ", i, j);
      hex(a + b); printf(" ");
      hex(a - b); printf(" ");
      hex(a * b); printf(" ");
      hex(a / b); printf(" ");
      printf("%d%d%d%d%d%d\n", a < b, a <= b, a > b, a >= b, a == b, a != b);
    }
  }
  for (int i = 0; i < 12; i++) {
    volatile long double a = vals[i];
    double d = (double)a;
    float f = (float)a;
    unsigned char db[8], fb[4];
    memcpy(db, &d, 8);
    memcpy(fb, &f, 4);
    printf("%d: ", i);
    for (int k = 7; k >= 0; k--) printf("%02x", db[k]);
    printf(" ");
    for (int k = 3; k >= 0; k--) printf("%02x", fb[k]);
    printf("\n");
  }
  static const char *lits[] = {"0", "-0", "1", "0.1", "1e4932", "1.18973149535723176502e+4932", "1.18973149535723176503e+4932",
    "3.64519953188247460253e-4951", "1.82259976594123730126e-4951", "1.8225997659412373013e-4951", "1e-4950", "1e-4952", "1e5000",
    "inf", "-inf", "nan", "123456789012345678901234567890", "18446744073709551615", "18446744073709551616", "0.000000000000000000001",
    "1.5e-3 rest", "  42", "+7.25e+2xyz", ".5", "5.", "1e", "e5", "-", ""};
  for (unsigned i = 0; i < sizeof lits / sizeof *lits; i++) {
    char *end;
    long double x = strtold(lits[i], &end);
    printf("[%s] ", lits[i]);
    hex(x);
    printf(" %d\n", (int)(end - lits[i]));
  }
  printf("%Lg %Lg %Lg %Lg\n", LDBL_MAX, LDBL_MIN, LDBL_EPSILON, (long double)LDBL_MANT_DIG);
  printf("%Le %Le\n", LDBL_MAX, LDBL_MIN);
  printf("%Lf %Lf %Lf\n", (long double)1e30L, (long double)-0.0L, (long double)1e-30L);
  printf("[%10.3Le] [%-12.2Lf] [%+Lg] [% Lg] [%#.0Lf] [%.0Lf] [%.0Lf]\n", 12345.6789L, 3.14159L, 2.5L, 2.5L, 3.0L, 0.5L, 1.5L);
  printf("[%Lg] [%Lg] [%LG] [%Lg]\n", 0.0001L, 0.00001L, 1e-10L, 123456789.0L);
  return 0;
}
