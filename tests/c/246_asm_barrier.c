int printf(const char *fmt, ...);
int main(void) {
  int x = 3, y = 4;
  asm volatile("" ::: "memory");
  __asm__ __volatile__("" : : : "memory", "cc");
  asm("" : "+r"(x));
  asm("" : : "r"(y));
  extern int ext(int) __asm__("" "ext");
  printf("%d %d\n", x, y);
  return x + y;
}
