#include <stdio.h>

static int depth;

static void done_int(int *p) { printf("done_int(%d) depth=%d\n", *p, depth); }
static void done_long(long *p) { printf("done_long(%ld)\n", *p); *p = -1; }
static void drop_ptr(char **p) { printf("drop_ptr(%s)\n", *p); }
static void leave(int *p) { depth -= *p; printf("leave %d depth=%d\n", *p, depth); }

typedef struct { int *counter; int step; } Guard;
static Guard guard_ctor(int *counter, int step) { Guard g = { counter, step }; *counter += step; printf("lock %d\n", *counter); return g; }
static void guard_dtor(Guard *g) { *g->counter -= g->step; printf("unlock %d\n", *g->counter); }
static void guard_ctx(Guard **p) { printf("ctx %d\n", *p != 0); }

struct Pair { int a; int b; };

static int plain(int n)
{
  int mark __attribute__((cleanup(done_int))) = n;
  mark += 1;
  return mark * 2;
}

static int early(int n)
{
  int one __attribute__((cleanup(done_int))) = 1;
  if (n < 0) return -1;
  {
    int two __attribute__((cleanup(done_int))) = 2;
    if (n == 0) return 0;
    two += n;
  }
  return n + one;
}

static int loops(int n)
{
  int total = 0;
  for (int i = 0; i < n; i++) {
    int step __attribute__((cleanup(done_int))) = i;
    if (i == 1) continue;
    if (i == 3) break;
    total += step;
  }
  return total;
}

static int gotos(int n)
{
  int r = 0;
  {
    long a __attribute__((cleanup(done_long))) = n;
    if (n > 5) goto out;
    r = (int)a * 2;
    goto inside;
inside:
    r += 1;
  }
  r += 100;
out:
  return r;
}

static void voidfn(int n)
{
  char *name __attribute__((cleanup(drop_ptr))) = "alpha";
  if (n) return;
  name = "beta";
}

static int guarded(int flag)
{
  int counter = 10;
  Guard g __attribute__((cleanup(guard_dtor))) = guard_ctor(&counter, 3), *unlock __attribute__((cleanup(guard_ctx))) = (Guard *)&counter;
  if (flag) return counter;
  counter += 100;
  return counter;
}

static int sw(int v)
{
  switch (v) {
  case 1: {
    int s1 __attribute__((cleanup(done_int))) = 11;
    if (v == 1) break;
    return -2;
  }
  case 2: {
    int s2 __attribute__((cleanup(done_int))) = 22;
    return s2;
  }
  default:
    return 0;
  }
  return 99;
}

static int shadow(void)
{
  int k = 5;
  int outer __attribute__((cleanup(done_int))) = k;
  {
    int outer = 77;
    if (outer > 50) return outer + k;
  }
  return 0;
}

static struct Pair retstruct(int x)
{
  int c __attribute__((cleanup(done_int))) = x;
  struct Pair p = { x, x * 2 };
  c = 0;
  return p;
}

static int twice(void)
{
  int d1 __attribute__((cleanup(leave))) = 1, d2 __attribute__((cleanup(leave))) = 2;
  depth += d1 + d2;
  return depth;
}

int main(void)
{
  int r1, r2, r3;
  r1 = plain(4);
  printf("plain %d\n", r1);
  r1 = early(-1);
  r2 = early(0);
  r3 = early(5);
  printf("early %d %d %d\n", r1, r2, r3);
  r1 = loops(6);
  printf("loops %d\n", r1);
  r1 = gotos(2);
  r2 = gotos(9);
  printf("gotos %d %d\n", r1, r2);
  voidfn(1);
  voidfn(0);
  r1 = guarded(1);
  r2 = guarded(0);
  printf("guarded %d %d\n", r1, r2);
  r1 = sw(1);
  r2 = sw(2);
  r3 = sw(3);
  printf("sw %d %d %d\n", r1, r2, r3);
  r1 = shadow();
  printf("shadow %d\n", r1);
  struct Pair p = retstruct(7);
  printf("pair %d %d\n", p.a, p.b);
  r1 = twice();
  printf("twice %d depth=%d\n", r1, depth);
  return 0;
}
