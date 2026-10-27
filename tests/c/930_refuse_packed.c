#include <stdio.h>
int g __attribute__((packed)) = 3;
int main(void) { printf("%d\n", g); return 0; }
