#ifndef C2ID_SETJMP_H
#define C2ID_SETJMP_H
typedef long jmp_buf[8];
typedef long sigjmp_buf[8];
int setjmp(jmp_buf env);
int _setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);
void _longjmp(jmp_buf env, int val);
#define sigsetjmp(e, m) _setjmp(e)
#define siglongjmp(e, v) _longjmp(e, v)
#endif
