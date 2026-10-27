#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  void *(*allocate)(size_t size);
  void (*deallocate)(void *pointer);
  void *(*reallocate)(void *pointer, size_t size);
} Hooks;

static Hooks hooks = { malloc, free, realloc };

typedef struct {
  const char *name;
  size_t (*fn)(const char *);
} Entry;

static const Entry entries[] = { { "strlen", strlen } };

static int count_if(int (*pred)(int), const char *s) {
  int n = 0;
  while (*s) {
    if (pred((unsigned char)*s)) n++;
    s++;
  }
  return n;
}

static double apply(double (*f)(double), double x) { return f(x); }

static int by_text(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }

int main(void) {
  char *p = hooks.allocate(8);
  char *words[4] = { "pear", "apple", "fig", "banana" };
  double (*fns[3])(double) = { sqrt, floor, fabs };
  int (*cmp)(const char *, const char *) = strcmp;
  int exp = 7;
  int i;
  void *(*mf)(size_t) = malloc;

  strcpy(p, "hello");
  p = hooks.reallocate(p, 64);
  strcat(p, " world");
  printf("%s %d\n", p, (int)entries[0].fn(p));
  hooks.deallocate(p);
  printf("%d %d\n", mf == malloc, hooks.allocate == malloc);
  printf("%s %d\n", entries[0].name, (int)entries[0].fn("seven77"));

  printf("digits %d alpha %d space %d\n", count_if(isdigit, "a1b22 c333"), count_if(isalpha, "a1b22 c333"), count_if(isspace, "a1b22 c333"));
  for (i = 0; i < 3; i++) printf("fn %d %.2f\n", i, apply(fns[i], i == 2 ? -2.5 : 6.25));
  printf("sin %.4f\n", apply(sin, 1.0));

  qsort(words, 4, sizeof words[0], by_text);
  for (i = 0; i < 4; i++) printf("%s ", words[i]);
  printf("\n");
  printf("cmp %d %d\n", cmp("a", "b") < 0, cmp("b", "b") == 0);
  printf("local %d\n", exp * 2);
  return 0;
}
