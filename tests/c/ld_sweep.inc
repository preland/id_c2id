/* The body of the randomized long double sweeps (231_ld_sweep_*.c): SEED and COUNT
   are defined by the file that includes this. Operand bit patterns are drawn from
   a generator of the cases that matter (normals over the whole exponent range,
   exponents near each other, denormals, zeros, infinities, NaNs, small integers,
   extreme exponents, patterns that round at a tie, the encodings the x87 refuses), each pair run through + - * /,
   the six comparisons and the narrowing conversions, and every result folded into
   one FNV-1a hash of its bytes. The oracle is the C compiler's x87, so the hash
   printed by the translated program has to be the compiler's. */
#include <stdio.h>
#include <string.h>

typedef unsigned long long u64;

static u64 st;
static u64 rnd(void) {
  st ^= st >> 12;
  st ^= st << 25;
  st ^= st >> 27;
  return st * 0x2545F4914F6CDD1DULL;
}

static u64 hash;
static void fold(unsigned char *p, int n) {
  for (int i = 0; i < n; i++) {
    hash ^= p[i];
    hash *= 0x100000001B3ULL;
  }
}

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

static void foldld(long double x) {
  unsigned char b[16];
  memcpy(b, &x, 16);
  fold(b, 10);
}

static void operand(int cls, u64 *m, unsigned *se) {
  unsigned sign = (unsigned)(rnd() & 1) << 15;
  switch (cls) {
  case 0:
    *m = rnd() | 0x8000000000000000ULL;
    *se = sign | (unsigned)(1 + rnd() % 32766);
    break;
  case 1:
    *m = rnd() | 0x8000000000000000ULL;
    *se = sign | (unsigned)(16383 - 70 + rnd() % 141);
    break;
  case 2:
    *m = (rnd() >> (rnd() % 64)) & 0x7FFFFFFFFFFFFFFFULL;
    if (*m == 0) *m = 1;
    *se = sign;
    break;
  case 3:
    *m = 0;
    *se = sign;
    break;
  case 4:
    *m = 0x8000000000000000ULL;
    *se = sign | 32767;
    break;
  case 5:
    *m = 0x8000000000000000ULL | ((rnd() >> (rnd() % 62)) & 0x3FFFFFFFFFFFFFFFULL);
    if (rnd() & 1) *m |= 0x4000000000000000ULL;
    if ((*m & 0x7FFFFFFFFFFFFFFFULL) == 0) *m |= 1;
    *se = sign | 32767;
    break;
  case 6: {
    u64 n = 1 + (rnd() >> (40 + rnd() % 24));
    int lz = 0;
    while (!(n << lz & 0x8000000000000000ULL)) lz++;
    *m = n << lz;
    *se = sign | (unsigned)(16383 + 63 - lz);
    break;
  }
  case 7:
    *m = rnd() | 0x8000000000000000ULL;
    *se = sign | (unsigned)((rnd() & 1) ? 1 + rnd() % 8 : 32766 - rnd() % 8);
    break;
  case 8: {
    static const u64 pats[] = {0x8000000000000000ULL, 0xFFFFFFFFFFFFFFFFULL, 0x8000000000000001ULL,
                               0xFFFFFFFFFFFFFFFEULL, 0x8000000000000800ULL, 0xC000000000000000ULL,
                               0x80000000000007FFULL, 0x8000000000000400ULL};
    *m = pats[rnd() % 8];
    *se = sign | (unsigned)(16383 - 66 + rnd() % 133);
    break;
  }
  case 10:
    *m = rnd() & 0x7FFFFFFFFFFFFFFFULL;
    if (*m == 0) *m = 5;
    *se = sign | (unsigned)(1 + rnd() % 32766);
    break;
  case 11:
    *m = (rnd() & 1) ? 0 : (rnd() & 0x7FFFFFFFFFFFFFFFULL);
    *se = sign | 32767;
    break;
  default: {
    int w = (rnd() & 1) ? 1100 : 170;
    *m = rnd() | 0x8000000000000000ULL;
    *se = sign | (unsigned)(16383 - w + rnd() % (2 * w + 20));
    if (rnd() % 4 == 0) *m &= 0xFFFFFFFFFFFFF800ULL;
    if (rnd() % 8 == 0) *m &= 0xFFFFFFFF00000000ULL;
    break;
  }
  }
}

static void derive(u64 *m, unsigned *se, int how) {
  switch (how) {
  case 0: *se ^= 0x8000; break;
  case 1:
    *se = (*se & 0x8000) | (((*se & 0x7FFF) + (unsigned)(rnd() % 3) - 1) & 0x7FFF);
    *m = (*m ^ (rnd() % 8)) | 0x8000000000000000ULL;
    break;
  case 2:
    *m = (*m + (rnd() % 4)) | 0x8000000000000000ULL;
    *se ^= (unsigned)(rnd() & 1) << 15;
    break;
  default:
    *se = (*se & 0x7FFF) | ((unsigned)(rnd() & 1) << 15);
    *m = rnd() | 0x8000000000000000ULL;
    break;
  }
}

static void step(void) {
  u64 am, bm;
  unsigned ase, bse;
  int ca = (int)(rnd() % 12), cb = (int)(rnd() % 12);
  operand(ca, &am, &ase);
  if (rnd() % 3 == 0) {
    bm = am;
    bse = ase;
    derive(&bm, &bse, (int)(rnd() % 4));
  } else {
    operand(cb, &bm, &bse);
  }
  volatile long double a = mk(am, ase), b = mk(bm, bse), r;
  r = a + b; foldld(r);
  r = a - b; foldld(r);
  r = a * b; foldld(r);
  r = a / b; foldld(r);
  int cmp = (a < b) | (a <= b) << 1 | (a > b) << 2 | (a >= b) << 3 | (a == b) << 4 | (a != b) << 5;
  fold((unsigned char *)&cmp, 1);
  double d = (double)a;
  float f = (float)a;
  fold((unsigned char *)&d, 8);
  fold((unsigned char *)&f, 4);
  r = -a; foldld(r);
}

static void integers(void) {
  for (int i = 0; i < 40; i++) {
    long long v = (long long)(rnd() >> (rnd() % 64));
    if (rnd() & 1) v = -v;
    unsigned long long uv = rnd() >> (rnd() % 64);
    volatile long double a = v, b = uv;
    foldld(a);
    foldld(b);
    long long back = (long long)a;
    unsigned long long uback = (unsigned long long)b;
    fold((unsigned char *)&back, 8);
    fold((unsigned char *)&uback, 8);
    long double h = a / 7 + b / 11;
    long long ht = (long long)h;
    fold((unsigned char *)&ht, 8);
  }
}

int main(void) {
  st = (u64)SEED * 0x9E3779B97F4A7C15ULL + 12345;
  hash = 0xCBF29CE484222325ULL;
  for (int i = 0; i < 20; i++) rnd();
  for (int i = 0; i < COUNT; i++) step();
  integers();
  printf("seed %d count %d hash %016llx\n", SEED, COUNT, hash);
  return 0;
}
