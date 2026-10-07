int g = 5;
extern typeof(g) g2;
typeof(g) g2 = 7;
int f(int a) { return a * 2; }
extern typeof(f) f;
int printf(const char *fmt, ...);
int main(void) { printf("%d %d %d\n", g, g2, f(g2)); return 0; }
