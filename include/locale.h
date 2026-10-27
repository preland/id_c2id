#ifndef C2ID_LOCALE_H
#define C2ID_LOCALE_H
struct lconv { char *decimal_point; };
struct lconv *localeconv(void);
#endif
