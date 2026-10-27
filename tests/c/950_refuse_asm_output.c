int main(void){ int x=0, y=7; __asm__("movl %1, %0" : "=r"(x) : "r"(y)); return x; }
