__extension__ typedef unsigned long long __u64;
typedef __u64 u64;
typedef u64 uint64_t;
void f(void *p){ uint64_t *q = (uint64_t *)p; *q = 1; }
int main(void){ unsigned long long z=0; f(&z); return (int)z; }
