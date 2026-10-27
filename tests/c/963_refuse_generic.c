int main(void) { int a = 1; return a + _Generic(a, int: 5, default: 6); }
