#include <stdio.h>

typedef __int128 s128;
typedef unsigned __int128 u128;

static void show(const char *tag, u128 v)
{
  printf("%s %016llx%016llx\n", tag, (unsigned long long)(v >> 64), (unsigned long long)v);
}

static u128 make(unsigned long long hi, unsigned long long lo)
{
  return ((u128)hi << 64) | lo;
}

static s128 smake(long long hi, unsigned long long lo)
{
  return (s128)(((u128)(unsigned long long)hi << 64) | lo);
}

static u128 mul_hi(unsigned long long a, unsigned long long b)
{
  return (u128)a * b;
}

static s128 passthru(s128 x, s128 y)
{
  return x * y - (x >> 3) + (y << 2);
}

struct Wide {
  char tag;
  u128 v;
  s128 s;
};

int main(void)
{
  u128 a = make(0x0123456789abcdefULL, 0xfedcba9876543210ULL);
  u128 b = make(0, 0x00000000deadbeefULL);
  u128 c = make(0xffffffffffffffffULL, 0xffffffffffffffffULL);
  s128 n = smake(-3, 7);
  show("a+b", a + b);
  show("a-b", a - b);
  show("b-a", b - a);
  show("a*b", a * b);
  show("a&b", a & b);
  show("a|b", a | b);
  show("a^b", a ^ b);
  show("~a", ~a);
  show("a<<5", a << 5);
  show("a<<70", a << 70);
  show("a>>9", a >> 9);
  show("a>>100", a >> 100);
  show("a/b", a / b);
  show("a%b", a % b);
  show("c/a", c / a);
  show("c%a", c % a);
  show("mulhi", mul_hi(0xffffffffffffffffULL, 0xfffffffffffffffeULL));
  show("n", (u128)n);
  show("-n", (u128)-n);
  show("n>>1", (u128)(n >> 1));
  show("n>>65", (u128)(n >> 65));
  show("n/3", (u128)(n / 3));
  show("n%3", (u128)(n % 3));
  show("n/-5", (u128)(n / -5));
  show("n%-5", (u128)(n % -5));
  show("pass", (u128)passthru(n, 11));
  printf("%d %d %d %d\n", a < b, a > b, n < 0, n > -n);
  printf("%d %d %d %d\n", a == a, a != b, c >= a, b <= a);
  printf("%d %d %d\n", !a, !(a - a), a && b);
  printf("%d %d\n", (int)a, (int)(a >> 64));
  printf("%llx %lld\n", (unsigned long long)a, (long long)n);
  printf("%u %d\n", (unsigned char)(a >> 3), (signed char)(a >> 3));
  s128 sx = -1;
  u128 ux = sx;
  show("sx", ux);
  show("short", (u128)(s128)(short)-2);
  show("ushort", (u128)(unsigned short)65535);
  u128 w = 5;
  w += 7;
  w *= 3;
  w -= 1;
  w <<= 66;
  w |= 3;
  w++;
  --w;
  show("w", w);
  u128 cnt = 0;
  for (u128 i = 1; i < 10; i++) cnt += i * i;
  show("cnt", cnt);
  struct Wide st;
  st.tag = 'x';
  st.v = a;
  st.s = n;
  show("st.v", st.v + (u128)st.tag);
  show("st.s", (u128)st.s);
  printf("%d %d\n", (int)sizeof(u128), (int)sizeof(struct Wide));
  u128 arr[3] = { 1, a, c };
  show("arr", arr[0] + arr[1] + arr[2]);
  u128 *p = &arr[1];
  show("p", *p ^ arr[2]);
  show("cond", (a > b) ? a : b);
  show("cond2", (a < b) ? a : b);
  return 0;
}
