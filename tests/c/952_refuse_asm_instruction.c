int main(void){ int x=0; asm volatile("" ::: "memory"); __asm__ __volatile__("" : : : "memory"); asm("nop"); return x; }
