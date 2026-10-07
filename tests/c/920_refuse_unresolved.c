#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int main(void) {
  char buf[8] = "a b";
  printf("%s %d\n", strtok(buf, " "), rand());
  return getpid();
}
