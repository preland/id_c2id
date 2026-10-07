#ifndef C2ID_STDLIB_H
#define C2ID_STDLIB_H
#include <stddef.h>
#include <string.h>
void *malloc(size_t n);
void *calloc(size_t n, size_t m);
void *realloc(void *p, size_t n);
void free(void *p);
void abort(void);
void exit(int code);
void _Exit(int code);
long strtol(const char *s, char **end, int base);
double strtod(const char *s, char **end);
static double atof(const char *s) { return strtod(s, (char **)0); }
char *getenv(const char *name);
static float strtof(const char *s, char **end) { return (float)strtod(s, end); }
static void (*__c2id_atexit_fns[32])(void);
static int __c2id_atexit_n;
static void __c2id_atexit_run(void) {
  while (__c2id_atexit_n > 0) {
    __c2id_atexit_n--;
    __c2id_atexit_fns[__c2id_atexit_n]();
  }
}
static int atexit(void (*fn)(void)) {
  if (__c2id_atexit_n >= 32) return -1;
  if (__c2id_atexit_n < 0) __c2id_atexit_run();
  __c2id_atexit_fns[__c2id_atexit_n++] = fn;
  return 0;
}
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 2147483647
static unsigned long strtoul(const char *s, char **end, int base) { return (unsigned long)strtol(s, end, base); }
static long long strtoll(const char *s, char **end, int base) { return (long long)strtol(s, end, base); }
static unsigned long long strtoull(const char *s, char **end, int base) { return (unsigned long long)strtol(s, end, base); }
static int atoi(const char *s) { return (int)strtol(s, (char **)0, 10); }
static long atol(const char *s) { return strtol(s, (char **)0, 10); }
static long long atoll(const char *s) { return (long long)strtol(s, (char **)0, 10); }
static int abs(int x) { return x < 0 ? -x : x; }
static long labs(long x) { return x < 0 ? -x : x; }
static long long llabs(long long x) { return x < 0 ? -x : x; }
static void qsort(void *base, size_t n, size_t sz, int (*cmp)(const void *, const void *)) {
  char *a = (char *)base;
  char *tmp;
  size_t width, lo, mid, hi, p, q, k;
  if (n < 2) return;
  tmp = (char *)malloc(n * sz);
  for (width = 1; width < n; width *= 2) {
    for (lo = 0; lo < n; lo += 2 * width) {
      mid = lo + width; if (mid > n) mid = n;
      hi = lo + 2 * width; if (hi > n) hi = n;
      p = lo; q = mid; k = lo;
      while (p < mid && q < hi) {
        if (cmp(a + q * sz, a + p * sz) < 0) { memcpy(tmp + k * sz, a + q * sz, sz); q++; }
        else { memcpy(tmp + k * sz, a + p * sz, sz); p++; }
        k++;
      }
      while (p < mid) { memcpy(tmp + k * sz, a + p * sz, sz); p++; k++; }
      while (q < hi) { memcpy(tmp + k * sz, a + q * sz, sz); q++; k++; }
    }
    memcpy(a, tmp, n * sz);
  }
  free(tmp);
}
static void *bsearch(const void *key, const void *base, size_t n, size_t sz, int (*cmp)(const void *, const void *)) {
  size_t lo = 0, hi = n, mid;
  const char *p;
  int c;
  while (lo < hi) {
    mid = lo + (hi - lo) / 2;
    p = (const char *)base + mid * sz;
    c = cmp(key, p);
    if (c == 0) return (void *)p;
    if (c < 0) hi = mid; else lo = mid + 1;
  }
  return (void *)0;
}
#endif
