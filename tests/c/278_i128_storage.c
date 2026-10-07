#include <stdio.h>
#include <string.h>

typedef unsigned __int128 u128;
typedef __int128 s128;

static u128 g_u = ((u128)3 << 100) | 77;
static s128 g_s = -5;
static u128 g_arr[3] = { 1, 2, (u128)1 << 127 };
static const s128 g_k = -((s128)1 << 90);
u128 g_zero;

struct Rec {
  int tag;
  u128 big;
  char tail[3];
  s128 neg;
};

static struct Rec g_rec = { 7, (u128)9 << 64, "ab", -2 };

static void show(const char *tag, u128 v)
{
  printf("%s %016llx %016llx\n", tag, (unsigned long long)(v >> 64), (unsigned long long)v);
}

static u128 sum3(u128 a, s128 b, u128 c)
{
  return a + (u128)b + c;
}

static u128 apply(u128 (*f)(u128, s128, u128), u128 x)
{
  return f(x, -1, 100);
}

static void inc_through(u128 *p, int n)
{
  for (int i = 0; i < n; i++) p[i] += (u128)(i + 1) << 70;
}

static struct Rec bump(struct Rec r)
{
  r.big += r.tag;
  r.neg *= 3;
  return r;
}

static u128 fact(int n)
{
  return n <= 1 ? (u128)1 : (u128)n * fact(n - 1);
}

int main(void)
{
  show("g_u", g_u);
  show("g_s", (u128)g_s);
  show("g_arr0", g_arr[0]);
  show("g_arr2", g_arr[2]);
  show("g_k", (u128)g_k);
  show("g_zero", g_zero);
  show("rec.big", g_rec.big);
  show("rec.neg", (u128)g_rec.neg);
  printf("%d %s %d\n", g_rec.tag, g_rec.tail, (int)sizeof(struct Rec));
  show("sum3", sum3(g_u, g_s, 1));
  show("apply", apply(sum3, g_u));
  inc_through(g_arr, 3);
  show("inc0", g_arr[0]);
  show("inc1", g_arr[1]);
  show("inc2", g_arr[2]);
  struct Rec b = bump(g_rec);
  show("bump.big", b.big);
  show("bump.neg", (u128)b.neg);
  show("rec.big after", g_rec.big);
  show("fact25", fact(25));
  show("fact34", fact(34));
  u128 local[2];
  local[0] = g_u;
  local[1] = local[0] * 3 + g_s;
  memcpy(&g_zero, &local[1], sizeof g_zero);
  show("memcpy", g_zero);
  u128 *p = &local[0];
  p += 1;
  show("ptr", *p - *(p - 1));
  printf("%d\n", (int)(p - local));
  unsigned long long parts[2];
  memcpy(parts, &local[1], 16);
  printf("%016llx %016llx\n", parts[1], parts[0]);
  s128 sx = (s128)g_u;
  sx = -sx;
  show("negswap", (u128)sx);
  if (g_u) printf("truthy\n");
  if (!g_zero) printf("zero\n"); else printf("nonzero\n");
  u128 cnt = 0;
  while (cnt < ((u128)1 << 66)) cnt += (u128)1 << 64;
  show("cnt", cnt);
  return 0;
}
