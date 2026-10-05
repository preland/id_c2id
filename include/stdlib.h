#ifndef C2ID_STDLIB_H
#define C2ID_STDLIB_H
#include <stddef.h>
void *malloc(size_t n);
void *calloc(size_t n, size_t m);
void *realloc(void *p, size_t n);
void free(void *p);
void abort(void);
void exit(int code);
int atoi(const char *s);
long strtol(const char *s, char **end, int base);
double strtod(const char *s, char **end);
int abs(int x);
void qsort(void *base, size_t n, size_t sz, int (*cmp)(const void *, const void *));
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define RAND_MAX 2147483647
#endif
