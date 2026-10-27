#include <math.h>
#include <stdio.h>

int main(void) {
  volatile double x = 0.7, y = 2.5;
  printf("%.15g %.15g %.15g\n", sqrt(x), cbrt(x), hypot(x, y));
  printf("%.15g %.15g %.15g\n", sin(x), cos(x), tan(x));
  printf("%.15g %.15g %.15g\n", asin(x), acos(x), atan(x));
  printf("%.15g %.15g %.15g\n", atan2(x, y), pow(x, y), fmod(y, x));
  printf("%.15g %.15g %.15g\n", sinh(x), cosh(x), tanh(x));
  printf("%.15g %.15g %.15g %.15g %.15g\n", exp(x), exp2(x), log(x), log2(x), log10(x));
  printf("%.15g %.15g %.15g %.15g %.15g\n", floor(-y), ceil(-y), round(-y), trunc(-y), fabs(-y));
  printf("%.15g %.15g %.15g\n", ldexp(x, 4), copysign(x, -1.0), fmin(x, y));
  printf("%.15g %d %d %d\n", fmax(x, y), isnan(x), isinf(x), isfinite(x));
  printf("%.15g %.15g\n", floor(2.5), ceil(2.5));
  printf("%d %d\n", isnan(NAN), isinf(INFINITY));
  printf("%.15g %.15g %.15g\n", M_PI, M_E, M_SQRT2);
  printf("%.15g\n", pow(2, 0.5) * sqrt(2.0));
  return 0;
}
