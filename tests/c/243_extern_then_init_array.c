extern int y[2];
int y[2] = { 7, 9 };
int printf(const char *fmt, ...);
int main(void) { printf("%d\n", y[1]); return 0; }
