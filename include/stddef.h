#ifndef C2ID_STDDEF_H
#define C2ID_STDDEF_H
typedef long ptrdiff_t;
typedef unsigned long size_t;
typedef int wchar_t;
#define NULL ((void *)0)
#define offsetof(t, m) __builtin_offsetof(t, m)
#endif
