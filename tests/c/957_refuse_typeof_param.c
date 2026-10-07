int f(int a, typeof(a) b) { return a + b; }
int main(void) { return f(1, 2); }
