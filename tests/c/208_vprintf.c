#include <stdio.h>
#include <stdarg.h>
static int fmt_to(char *buf, size_t n, const char *fmt, ...) { va_list ap; int r; va_start(ap, fmt); r = vsnprintf(buf, n, fmt, ap); va_end(ap); return r; }
static void log_line(const char *tag, const char *fmt, ...) { va_list ap; printf("[%s] ", tag); va_start(ap, fmt); vprintf(fmt, ap); va_end(ap); printf("\n"); }
static void err_line(const char *fmt, ...) { va_list ap; va_start(ap, fmt); vfprintf(stdout, fmt, ap); va_end(ap); fputc('\n', stdout); }
static int sfmt(char *buf, const char *fmt, ...) { va_list ap; int r; va_start(ap, fmt); r = vsprintf(buf, fmt, ap); va_end(ap); return r; }
int main(void) {
  char buf[64], big[16];
  int r = fmt_to(buf, sizeof buf, "%d-%s-%5.2f-%c-%lx", 42, "str", 3.14159, 'z', 255L);
  printf("%d %s\n", r, buf);
  r = fmt_to(big, sizeof big, "%s", "a rather long string indeed");
  printf("%d %s\n", r, big);
  log_line("tag", "%d + %d = %d", 1, 2, 3);
  err_line("%s=%ld", "k", 1L << 40);
  r = sfmt(buf, "%x/%o", 255, 8);
  printf("%d %s\n", r, buf);
  return 0;
}
