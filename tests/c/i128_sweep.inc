/* The body of the randomized 128-bit sweeps (277_i128_sweep_*.c): SEED, COUNT and one
   of SWEEP_ARITH, SWEEP_SHIFT, SWEEP_DIV and SWEEP_MIX are defined by the file that
   includes this. Operand bit patterns are drawn from a generator of the cases that
   matter (full-width random, small values, values around the 2^64 boundary, all
   ones, powers of two, the signed extremes, zero), each pair run through the
   operators of the group, and every result folded into one FNV-1a hash of its
   bytes. The oracle is the C compiler's own __int128, so the hash printed by the
   translated program has to be the compiler's. */
#include <stdio.h>
#include <string.h>

typedef unsigned long long u64;
typedef unsigned __int128 u128;
typedef __int128 s128;

static u64 st;
static u64 rnd(void) {
  st ^= st >> 12;
  st ^= st << 25;
  st ^= st >> 27;
  return st * 0x2545F4914F6CDD1DULL;
}

static u64 hash;
static void fold64(u64 v) {
  for (int i = 0; i < 8; i++) {
    hash ^= (v >> (8 * i)) & 255;
    hash *= 0x100000001B3ULL;
  }
}

static void fold(u128 v) {
  fold64((u64)v);
  fold64((u64)(v >> 64));
}

static u128 operand(void) {
  u64 hi, lo;
  switch (rnd() % 9) {
  case 0: hi = rnd(); lo = rnd(); break;
  case 1: hi = 0; lo = rnd() % 1000003; break;
  case 2: hi = 0; lo = rnd(); break;
  case 3: hi = rnd() % 5; lo = rnd(); break;
  case 4: hi = ~0ULL; lo = rnd(); break;
  case 5: hi = 0x8000000000000000ULL; lo = rnd() % 3; break;
  case 6: hi = 0x7fffffffffffffffULL; lo = ~0ULL - rnd() % 3; break;
  case 7: hi = (rnd() & 1) ? 1ULL << (rnd() % 64) : 0; lo = (rnd() & 1) ? 1ULL << (rnd() % 64) : 0; break;
  default: hi = 0; lo = rnd() % 5; break;
  }
  return ((u128)hi << 64) | lo;
}

#ifdef SWEEP_ARITH
static void pair(u128 a, u128 b) {
  s128 sa = (s128)a, sb = (s128)b;
  fold(a + b);
  fold(a - b);
  fold(a * b);
  fold(a & b);
  fold(a | b);
  fold(a ^ b);
  fold(~a);
  fold((u128)-sa);
  fold(a < b);
  fold(a <= b);
  fold(a > b);
  fold(a >= b);
  fold(a == b);
  fold(a != b);
  fold(sa < sb);
  fold(sa <= sb);
  fold(sa > sb);
  fold(sa >= sb);
  fold(!a);
  fold(a && b);
  fold(a || b);
  fold(a ? b : a);
}
#endif

#ifdef SWEEP_SHIFT
static void pair(u128 a, u128 b) {
  s128 sa = (s128)a;
  int n = (int)(b & 127);
  fold(a << n);
  fold(a >> n);
  fold((u128)(sa >> n));
  fold(a << (n & 63));
  fold((u128)(sa >> (n | 64)));
  fold(a >> (127 - n));
  fold((u128)((s128)1 << n));
}
#endif

#ifdef SWEEP_DIV
static void pair(u128 a, u128 b) {
  s128 sa = (s128)a, sb = (s128)b;
  if (b != 0) {
    fold(a / b);
    fold(a % b);
  }
  if (sb != 0 && !(sa == (s128)((u128)1 << 127) && sb == -1)) {
    fold((u128)(sa / sb));
    fold((u128)(sa % sb));
  }
}
#endif

#ifdef SWEEP_MIX
static void pair(u128 a, u128 b) {
  s128 sa = (s128)a;
  u64 w = (u64)b;
  long long sw = (long long)w;
  int i = (int)w;
  unsigned short us = (unsigned short)w;
  signed char c = (signed char)w;
  fold(a + w);
  fold(a - sw);
  fold(a * i);
  fold(a & us);
  fold((u128)(sa + c));
  fold((u128)(sa * sw));
  fold((u64)a);
  fold((unsigned)a);
  fold((u128)(short)a);
  fold((u128)(signed char)(a >> 5));
  fold((u128)(long long)sa);
  fold((u128)(s128)i);
  fold((u128)(u128)(unsigned)w);
  fold((u128)(_Bool)a);
  fold(a + (u128)(sw < 0 ? -1 : 1));
  u128 t = a;
  t += w;
  t -= i;
  t *= 3;
  t ^= b;
  t <<= 3;
  t >>= 1;
  t |= us;
  t &= ~(u128)c;
  fold(t);
  t++;
  ++t;
  t--;
  fold(t);
  s128 q = sa;
  q >>= 2;
  q += sw;
  q++;
  fold((u128)q);
}
#endif

int main(void) {
  st = SEED;
  hash = 0xCBF29CE484222325ULL;
  for (int k = 0; k < COUNT; k++) {
    u128 a = operand();
    u128 b = operand();
    pair(a, b);
  }
  printf("%d %llx\n", COUNT, hash);
  return 0;
}
