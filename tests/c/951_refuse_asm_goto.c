int printf(const char *f, ...);
int main(void){ int x = 1; asm goto("jmp %l0" : : : : out); x = 2; out: printf("%d\n", x); return 0; }
