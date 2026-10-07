#include <stdio.h>
typedef struct { long a[2]; } Small;
typedef struct { long a[100]; } Big;
typedef int Word;
static size_t sz_small(void) { typedef struct { char c; Small t; } t_a; return sizeof(t_a); }
static size_t sz_big(void) { typedef struct { char c; Big t; } t_a; return sizeof(t_a); }
static size_t tag_a(void) { struct T { char c; char d; }; return sizeof(struct T); }
static size_t tag_b(void) { struct T { long x[3]; }; return sizeof(struct T); }
static int enum_a(void) { enum { K = 3 }; return K; }
static int enum_b(void) { enum { K = 9 }; return K; }
static size_t inner(void) { typedef char Word; size_t n = sizeof(Word); { typedef long Word; n = n * 100 + sizeof(Word); } return n * 100 + sizeof(Word); }
int main(void) {
  Word w = 5;
  printf("%d %d\n", (int)sz_small(), (int)sz_big());
  printf("%d %d %d %d\n", (int)tag_a(), (int)tag_b(), enum_a(), enum_b());
  printf("%d %d\n", (int)inner(), (int)sizeof(Word) + w);
  return 0;
}
