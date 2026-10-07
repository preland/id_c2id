#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  double a, b, c, d;
  float f, g;
  int r, k;
  char word[16];
  char *end;
  r = sscanf("3.14 -2e3 .5 7.", "%lf %lf %lf %lf", &a, &b, &c, &d);
  printf("%d %.17g %.17g %.17g %.17g\n", r, a, b, c, d);
  r = sscanf("1.5 2.25", "%f %f", &f, &g);
  printf("%d %.9g %.9g\n", r, f, g);
  r = sscanf("12 3.5x", "%d %lf", &k, &a);
  printf("%d %d %.17g\n", r, k, a);
  r = sscanf("abc", "%lf", &a);
  printf("%d\n", r);
  r = sscanf("inf -inf nan", "%lf %lf %lf", &a, &b, &c);
  printf("%d %f %f %f\n", r, a, b, c);
  r = sscanf("1e500 1e-500", "%lf %lf", &a, &b);
  printf("%d %g %g\n", r, a, b);
  r = sscanf("  42.5e1xyz", "%lf%s", &a, word);
  printf("%d %.17g %s\n", r, a, word);
  printf("%.17g\n", strtod("  -0.1e+2abc", &end));
  printf("[%s]\n", end);
  printf("%.17g\n", strtod("0.1", 0));
  printf("%.17g\n", atof("2.5e-3"));
  a = strtod("junk", &end);
  printf("%.17g [%s]\n", a, end);
  a = strtod("1e", &end);
  printf("%.17g [%s]\n", a, end);
  a = strtod("1.5e+", &end);
  printf("%.17g [%s]\n", a, end);
  a = strtod("-.25z", &end);
  printf("%.17g [%s]\n", a, end);
  a = strtod("INF", &end);
  printf("%.17g [%s]\n", a, end);
  printf("%.17g\n", strtod("123456789012345678901234567890", 0));
  printf("%.17g\n", strtod("4.9e-324", 0));
  return 0;
}
