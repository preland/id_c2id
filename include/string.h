#ifndef C2ID_STRING_H
#define C2ID_STRING_H
#include <stddef.h>
void *memcpy(void *d, const void *s, size_t n);
void *memmove(void *d, const void *s, size_t n);
void *memset(void *d, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
void *memchr(const void *s, int c, size_t n);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strcpy(char *d, const char *s);
char *strncpy(char *d, const char *s, size_t n);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *h, const char *n);
char *strcat(char *d, const char *s);
char *strncat(char *d, const char *s, size_t n);
char *strdup(const char *s);
static char *strpbrk(const char *s, const char *set) {
  for (; *s; s++) {
    const char *p;
    for (p = set; *p; p++) if (*p == *s) return (char *)s;
  }
  return (char *)0;
}
static size_t strspn(const char *s, const char *set) {
  size_t n = 0;
  for (; s[n]; n++) {
    const char *p;
    for (p = set; *p && *p != s[n]; p++) {}
    if (!*p) break;
  }
  return n;
}
static size_t strcspn(const char *s, const char *set) {
  size_t n = 0;
  for (; s[n]; n++) {
    const char *p;
    for (p = set; *p && *p != s[n]; p++) {}
    if (*p) break;
  }
  return n;
}
static char *strerror(int e) {
  switch (e) {
  case 0: return "Success";
  case 1: return "Operation not permitted";
  case 2: return "No such file or directory";
  case 3: return "No such process";
  case 4: return "Interrupted system call";
  case 5: return "Input/output error";
  case 9: return "Bad file descriptor";
  case 11: return "Resource temporarily unavailable";
  case 12: return "Cannot allocate memory";
  case 13: return "Permission denied";
  case 17: return "File exists";
  case 20: return "Not a directory";
  case 21: return "Is a directory";
  case 22: return "Invalid argument";
  case 24: return "Too many open files";
  case 28: return "No space left on device";
  case 32: return "Broken pipe";
  case 33: return "Numerical argument out of domain";
  case 34: return "Numerical result out of range";
  }
  return "Unknown error";
}
/* The address of a function the runtime provides is the address of the wrapper of
   the same name: a pointer is a place in the program's own function table, which
   holds only functions the program defines (mid/check/rtval.id). */
static void *__c2id_w_memcpy(void *d, const void *s, size_t n) { return memcpy(d, s, n); }
static void *__c2id_w_memmove(void *d, const void *s, size_t n) { return memmove(d, s, n); }
static void *__c2id_w_memset(void *d, int c, size_t n) { return memset(d, c, n); }
static int __c2id_w_memcmp(const void *a, const void *b, size_t n) { return memcmp(a, b, n); }
static void *__c2id_w_memchr(const void *s, int c, size_t n) { return memchr(s, c, n); }
static size_t __c2id_w_strlen(const char *s) { return strlen(s); }
static int __c2id_w_strcmp(const char *a, const char *b) { return strcmp(a, b); }
static int __c2id_w_strncmp(const char *a, const char *b, size_t n) { return strncmp(a, b, n); }
static char *__c2id_w_strcpy(char *d, const char *s) { return strcpy(d, s); }
static char *__c2id_w_strncpy(char *d, const char *s, size_t n) { return strncpy(d, s, n); }
static char *__c2id_w_strchr(const char *s, int c) { return strchr(s, c); }
static char *__c2id_w_strrchr(const char *s, int c) { return strrchr(s, c); }
static char *__c2id_w_strstr(const char *h, const char *n) { return strstr(h, n); }
static char *__c2id_w_strcat(char *d, const char *s) { return strcat(d, s); }
static char *__c2id_w_strncat(char *d, const char *s, size_t n) { return strncat(d, s, n); }
static char *__c2id_w_strdup(const char *s) { return strdup(s); }
#endif
