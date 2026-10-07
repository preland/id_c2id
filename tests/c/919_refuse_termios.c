#include <stdio.h>
#include <termios.h>
int main(void) {
  struct termios t;
  if (tcgetattr(0, &t) != 0) return 1;
  t.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(0, TCSANOW, &t);
  return 0;
}
