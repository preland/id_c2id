#ifndef C2ID_SETJMP_H
#define C2ID_SETJMP_H
typedef long jmp_buf[8];
int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
#endif
