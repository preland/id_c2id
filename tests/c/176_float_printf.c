#include <stdio.h>
#include <stdlib.h>

int main(void) {
  double v[] = {0.0, -0.0, 1.0, -1.5, 0.1, 0.125, 0.375, 2.5, 3.5, 1e-5, 123456789.0, 1e21,
                1e100, 1.7976931348623157e308, 4.9e-324, 2.2250738585072014e-308, 0.5, 100.0,
                999999.5, 0.000123456, 12345.6789, 1.0 / 3, 9.9999995, 0.05, 99.5, 1e15, 123456.7};
  int n = sizeof(v) / sizeof(v[0]);
  int i;
  for (i = 0; i < n; i++) {
    printf("%f|%.2f|%e|%.3e|%g|%.10g|%G|%E\n", v[i], v[i], v[i], v[i], v[i], v[i], v[i], v[i]);
    printf("%10.3f|%-10.1f|%+f|% f|%010.2f|%#.0f|%.0f|%#g|%.0e|%#.0e\n", v[i], v[i], v[i], v[i], v[i], v[i], v[i], v[i], v[i], v[i]);
  }
  printf("%.20f\n", 0.1);
  printf("%.30e\n", 1.0 / 3);
  printf("%.0f %.0f %.0f %.0f\n", 0.5, 1.5, 2.5, 3.5);
  printf("%.1f %.1f %.1f\n", 0.05, 0.15, 0.25);
  printf("%5.1f|%-8.3e|%8.2g|\n", 2.25, 12345.678, 0.000012345);
  printf("%g %g %g %g\n", 100000.0, 1000000.0, 0.0001, 0.00001);
  printf("%f %F %e %g\n", 1.0f, 2.5f, 3.0f, 4.0f);
  double nn = strtod("nan", 0);
  double in = strtod("inf", 0);
  printf("%f %f %f %f\n", in, -in, nn, -nn);
  printf("%e %E %g %G\n", in, in, nn, nn);
  printf("%8f|%-8f|%08f|%+f\n", in, in, in, nn);
  return 0;
}
