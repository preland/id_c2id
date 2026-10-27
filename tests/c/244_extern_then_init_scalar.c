extern int x;
int get(void) { return x; }
int x = 5;
int printf(const char *fmt, ...);
int main(void) { printf("%d\n", get()); return 0; }
