int printf(const char *f, ...);
int f(int n){ int a[n]; int i, s = 0; for (i = 0; i < n; i++) a[i] = i * 2; for (i = 0; i < n; i++) s += a[i]; return s; }
int main(void){ printf("%d %d\n", f(5), f(1)); return 0; }
