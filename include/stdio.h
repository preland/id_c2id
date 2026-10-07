#ifndef C2ID_STDIO_H
#define C2ID_STDIO_H
#include <stddef.h>
#include <stdarg.h>
typedef struct _IO_FILE FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
#define EOF (-1)
int printf(const char *fmt, ...);
int fprintf(FILE *f, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int snprintf(char *buf, size_t n, const char *fmt, ...);
int vsnprintf(char *buf, size_t n, const char *fmt, va_list ap);
int puts(const char *s);
int putchar(int c);
int fputs(const char *s, FILE *f);
int fputc(int c, FILE *f);
int fgetc(FILE *f);
int ungetc(int c, FILE *f);
#define getchar() fgetc(stdin)
#define getc(f) fgetc(f)
#define putc(c, f) fputc(c, f)
int fflush(FILE *f);
FILE *fopen(const char *path, const char *mode);
FILE *tmpfile(void);
int remove(const char *path);
int rename(const char *from, const char *to);
int fclose(FILE *f);
size_t fread(void *p, size_t sz, size_t n, FILE *f);
size_t fwrite(const void *p, size_t sz, size_t n, FILE *f);
char *fgets(char *s, int n, FILE *f);
int sscanf(const char *s, const char *fmt, ...);
int vsprintf(char *buf, const char *fmt, va_list ap);
int vprintf(const char *fmt, va_list ap);
int vfprintf(FILE *f, const char *fmt, va_list ap);
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
static long ftell(FILE *fp) {
  long *f = (long *)fp;
  if ((unsigned long)fp <= 3 || f[3] != 0 && f[3] != 2) return -1L;
  return f[2];
}
static int fseek(FILE *fp, long off, int whence) {
  long *f = (long *)fp;
  long p;
  if ((unsigned long)fp <= 3 || f[3] != 0 && f[3] != 2) return -1;
  p = off;
  if (whence == 1) p = f[2] + off;
  if (whence == 2) p = f[1] + off;
  if (p < 0 || p > f[1]) return -1;
  f[2] = p;
  return 0;
}
static void rewind(FILE *fp) { fseek(fp, 0L, 0); }
static int __c2id_w_putchar(int c) { return putchar(c); }
static int __c2id_w_puts(const char *s) { return puts(s); }
static int __c2id_w_fputs(const char *s, FILE *f) { return fputs(s, f); }
static int __c2id_w_fputc(int c, FILE *f) { return fputc(c, f); }
static int __c2id_w_fgetc(FILE *f) { return fgetc(f); }
static int __c2id_w_ungetc(int c, FILE *f) { return ungetc(c, f); }
static int __c2id_w_fflush(FILE *f) { return fflush(f); }
static FILE *__c2id_w_fopen(const char *path, const char *mode) { return fopen(path, mode); }
static int __c2id_w_fclose(FILE *f) { return fclose(f); }
static size_t __c2id_w_fread(void *p, size_t sz, size_t n, FILE *f) { return fread(p, sz, n, f); }
static size_t __c2id_w_fwrite(const void *p, size_t sz, size_t n, FILE *f) { return fwrite(p, sz, n, f); }
static char *__c2id_w_fgets(char *s, int n, FILE *f) { return fgets(s, n, f); }
static int __c2id_w_remove(const char *path) { return remove(path); }
static int __c2id_w_rename(const char *from, const char *to) { return rename(from, to); }
#endif
