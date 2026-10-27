#ifndef C2ID_STDARG_H
#define C2ID_STDARG_H
typedef struct { char *cur; char *end; } __c2id_va_cell;
typedef __c2id_va_cell va_list[1];
#define __c2id_va_slot(t) ((sizeof(t) + 7) & ~7)
#define va_start(v, l) ((v)->cur = __va, (v)->end = *(char **)(__va - 8))
#define va_end(v) ((void)0)
#define va_arg(v, t) (*(t *)(((v)->cur += __c2id_va_slot(t)) - __c2id_va_slot(t)))
#define va_copy(d, s) ((d)->cur = (s)->cur, (d)->end = (s)->end)
#endif
