#ifndef C2ID_CTYPE_H
#define C2ID_CTYPE_H
int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
int isspace(int c);
int isupper(int c);
int islower(int c);
int isxdigit(int c);
int isprint(int c);
int isgraph(int c);
int iscntrl(int c);
int ispunct(int c);
int toupper(int c);
int tolower(int c);
static int __c2id_w_isdigit(int c) { return isdigit(c); }
static int __c2id_w_isalpha(int c) { return isalpha(c); }
static int __c2id_w_isalnum(int c) { return isalnum(c); }
static int __c2id_w_isspace(int c) { return isspace(c); }
static int __c2id_w_isupper(int c) { return isupper(c); }
static int __c2id_w_islower(int c) { return islower(c); }
static int __c2id_w_isxdigit(int c) { return isxdigit(c); }
static int __c2id_w_isprint(int c) { return isprint(c); }
static int __c2id_w_isgraph(int c) { return isgraph(c); }
static int __c2id_w_iscntrl(int c) { return iscntrl(c); }
static int __c2id_w_ispunct(int c) { return ispunct(c); }
static int __c2id_w_toupper(int c) { return toupper(c); }
static int __c2id_w_tolower(int c) { return tolower(c); }
#endif
