#include <stdio.h>
#include <stddef.h>

struct S { long a; char b; short c[3]; };
struct T { struct S s; int tag; };
long g;
unsigned char arr[5];
struct S gs;
double d = 1.5;
int f(int x) { return x + 1; }

typeof(arr[0]) at_first = 200;
typeof(gs.c) cs = { 1, 2, 3 };
typeof(&gs) pgs = &gs;
typeof(f(3)) fres = 9;
extern typeof(d) d2;
typeof(d) d2 = 2.25;
typeof(((struct S *)0)->a) member_copy = 77;
typeof(sizeof(struct S)) sz = sizeof(struct S);

struct U { typeof(((struct S *)0)->b) b2; typeof(((struct S *)0)->c) c2; typeof(g) g2; };

static long take(typeof(((struct S *)0)->a) v, typeof(g) w, typeof(((struct T *)0)->s.c[1]) z)
{
  return v + w + z;
}

#define KIND(x) __builtin_choose_expr(__builtin_types_compatible_p(typeof(x), long), 1, __builtin_choose_expr(__builtin_types_compatible_p(typeof(x), double), 2, 3))

int main(void)
{
  struct U u;
  u.b2 = 'k';
  u.c2[2] = 9;
  u.g2 = 123456789012L;
  printf("%d %d %d %d %d %d\n", at_first, cs[2], (int)sizeof(cs), fres, (int)member_copy, (int)sz);
  printf("%g %d %d\n", d2, (int)sizeof(u), (int)offsetof(struct U, g2));
  printf("%c %d %ld\n", u.b2, u.c2[2], u.g2);
  gs.a = 5;
  printf("%ld\n", take(gs.a, 10, 3));
  pgs->c[1] = 4;
  printf("%d %d %d %d\n", gs.c[1], KIND(g), KIND(d), KIND(at_first));
  return 0;
}
