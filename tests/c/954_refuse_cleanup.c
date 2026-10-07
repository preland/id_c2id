void f(int *p){} int main(void){ int x __attribute__((cleanup(f))) = 1; return x; }
