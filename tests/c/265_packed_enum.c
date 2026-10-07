#include <stdio.h>
#include <stddef.h>

enum Small { S0, S1, S2 = 200 } __attribute__((__packed__));
enum __attribute__((packed)) Wide { W0 = 1000, W1, W2 = 60000 };
enum Neg { N0 = -3, N1, N2 = 100 } __attribute__((packed));
enum NegWide { M0 = -300, M1 = 5 } __attribute__((packed));
enum Big { B0 = 70000, B1 } __attribute__((packed));
enum Plain { P0, P1 };
typedef enum { T0 = 7, T1 } __attribute__((packed)) Tdef;

struct Rec { char c; enum Small s; enum Wide w; enum Neg n; Tdef t; enum Plain p; };
struct __attribute__((packed)) PRec { enum Small s; enum Big b; enum NegWide m; char tail; };

static int classify(enum Neg n)
{
  if (n < 0) return -1;
  if (n > N1) return 2;
  return 0;
}

int main(void)
{
  printf("%d %d %d %d %d %d %d\n", (int)sizeof(enum Small), (int)sizeof(enum Wide), (int)sizeof(enum Neg), (int)sizeof(enum NegWide), (int)sizeof(enum Big), (int)sizeof(enum Plain), (int)sizeof(Tdef));
  printf("%d %d %d\n", (int)sizeof(struct Rec), (int)sizeof(struct PRec), (int)offsetof(struct Rec, p));
  struct Rec r;
  r.c = 'q';
  r.s = S2;
  r.w = W2;
  r.n = N0;
  r.t = T1;
  r.p = P1;
  printf("%c %d %d %d %d %d\n", r.c, r.s, r.w, r.n, r.t, r.p);
  r.n = (enum Neg)(r.n + 1);
  printf("%d %d %d %d\n", r.n, classify(r.n), classify(N2), classify(N1));
  enum Small s = S2;
  s = (enum Small)(s + 100);
  printf("%d\n", s);
  enum Small arr[4] = { S0, S1, S2, S1 };
  int total = 0;
  for (int k = 0; k < 4; k++) total += arr[k];
  printf("%d %d\n", total, (int)sizeof arr);
  struct PRec pr;
  pr.s = S1;
  pr.b = B1;
  pr.m = M0;
  pr.tail = 'z';
  printf("%d %d %d %c\n", pr.s, pr.b, pr.m, pr.tail);
  return 0;
}
