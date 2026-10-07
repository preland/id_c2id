#include <stdio.h>

struct S { long a; char b; };
struct Bits { unsigned short lo : 4, mid : 3; unsigned short hi : 9; } __attribute__((packed));
struct Seg { int __seg_gs *cpu; int tag; };
int __seg_gs per_cpu_counter;
int runtime_len(int);
struct Big { char head; char tail[(__builtin_constant_p(4096ULL) ? 12 : runtime_len(3))]; };
struct Holder { struct Big inner[2]; int count; };

static void cleanup_helper(int *p) { *p = 0; }

static inline int unreached_cleanup(int v)
{
  int guard __attribute__((cleanup(cleanup_helper))) = v;
  return guard + 1;
}

static inline unsigned long unreached_int128(unsigned long a, unsigned long b)
{
  unsigned __int128 w = (unsigned __int128)a * b;
  return (unsigned long)(w >> 64);
}

static inline int unreached_seg(int c)
{
  int __seg_gs *p = &per_cpu_counter;
  if (c) return *p;
  return 0;
}

static inline int unreached_vla(int n)
{
  int arr[n];
  arr[0] = n;
  return arr[0];
}

static inline int unreached_typeof(int a, long b)
{
  return __builtin_choose_expr(__builtin_types_compatible_p(typeof(a), long), 1, 2) + (int)sizeof(typeof(b));
}

static inline int unreached_seg_member(struct Seg *s)
{
  return s->tag + *s->cpu;
}

struct Tbl { int (*fn)(int); };
static const struct Tbl unreferenced_table = { unreached_cleanup };

static int used_one(int x) { return x * 3; }

int main(void)
{
  struct S s = { 5, 'x' };
  struct Bits b;
  b.lo = 9;
  b.mid = 5;
  b.hi = 300;
  printf("%d %d %d %d\n", used_one(7), (int)sizeof(s), (int)sizeof(b), b.hi + b.mid + b.lo);
  return 0;
}
