#ifndef C2ID_MATH_H
#define C2ID_MATH_H
double sqrt(double x);
double cbrt(double x);
double hypot(double x, double y);
double sin(double x);
double cos(double x);
double tan(double x);
double asin(double x);
double acos(double x);
double atan(double x);
double atan2(double y, double x);
double pow(double x, double y);
double fmod(double x, double y);
double sinh(double x);
double cosh(double x);
double tanh(double x);
double exp(double x);
double exp2(double x);
double log(double x);
double log2(double x);
double log10(double x);
double ldexp(double x, int e);
double floor(double x);
double ceil(double x);
double round(double x);
double trunc(double x);
double fabs(double x);
double copysign(double x, double y);
double fmin(double x, double y);
double fmax(double x, double y);
int isnan(double x);
int isinf(double x);
int isfinite(double x);
double __builtin_nan(const char *s);
double __builtin_inf(void);
#define NAN __builtin_nan("")
#define INFINITY __builtin_inf()
#define HUGE_VAL __builtin_inf()
#define M_E 2.7182818284590452354
#define M_LOG2E 1.4426950408889634074
#define M_LOG10E 0.43429448190325182765
#define M_LN2 0.69314718055994530942
#define M_LN10 2.30258509299404568402
#define M_PI 3.14159265358979323846
#define M_PI_2 1.57079632679489661923
#define M_PI_4 0.78539816339744830962
#define M_1_PI 0.31830988618379067154
#define M_2_PI 0.63661977236758134308
#define M_SQRT2 1.41421356237309504880
#define M_SQRT1_2 0.70710678118654752440
static double __c2id_w_sqrt(double x) { return sqrt(x); }
static double __c2id_w_cbrt(double x) { return cbrt(x); }
static double __c2id_w_hypot(double x, double y) { return hypot(x, y); }
static double __c2id_w_sin(double x) { return sin(x); }
static double __c2id_w_cos(double x) { return cos(x); }
static double __c2id_w_tan(double x) { return tan(x); }
static double __c2id_w_asin(double x) { return asin(x); }
static double __c2id_w_acos(double x) { return acos(x); }
static double __c2id_w_atan(double x) { return atan(x); }
static double __c2id_w_atan2(double y, double x) { return atan2(y, x); }
static double __c2id_w_pow(double x, double y) { return pow(x, y); }
static double __c2id_w_fmod(double x, double y) { return fmod(x, y); }
static double __c2id_w_sinh(double x) { return sinh(x); }
static double __c2id_w_cosh(double x) { return cosh(x); }
static double __c2id_w_tanh(double x) { return tanh(x); }
static double __c2id_w_exp(double x) { return exp(x); }
static double __c2id_w_exp2(double x) { return exp2(x); }
static double __c2id_w_log(double x) { return log(x); }
static double __c2id_w_log2(double x) { return log2(x); }
static double __c2id_w_log10(double x) { return log10(x); }
static double __c2id_w_ldexp(double x, int e) { return ldexp(x, e); }
static double __c2id_w_floor(double x) { return floor(x); }
static double __c2id_w_ceil(double x) { return ceil(x); }
static double __c2id_w_round(double x) { return round(x); }
static double __c2id_w_trunc(double x) { return trunc(x); }
static double __c2id_w_fabs(double x) { return fabs(x); }
static double __c2id_w_copysign(double x, double y) { return copysign(x, y); }
static double __c2id_w_fmin(double x, double y) { return fmin(x, y); }
static double __c2id_w_fmax(double x, double y) { return fmax(x, y); }
static int __c2id_w_isnan(double x) { return isnan(x); }
static int __c2id_w_isinf(double x) { return isinf(x); }
static int __c2id_w_isfinite(double x) { return isfinite(x); }
#endif
