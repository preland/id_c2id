typedef struct { unsigned long sig[1]; } sigset_t;
static inline void sigemptyset(sigset_t *set) { set->sig[0] = 0; }
int main(void){ sigset_t s; s.sig[0] = 5; sigemptyset(&s); return (int)s.sig[0]; }
