#include <stdio.h>
#include <string.h>

int main(void) {
  char name[8] = "abcdefg";
  char raw[4] = {'w', 'x', 'y', 'z'};
  int n = 3, w = 6;
  printf("[%5d][%-5d][%05d][%+d][% d][%.3d][%8.3d][%-8.3d|][%08.3d]\n", 42, 42, 42, 42, 42, 7, 7, 7, 7);
  printf("[%5d][%-5d][%05d][%+d][% d][%.3d][%8.3d]\n", -42, -42, -42, -42, -42, -7, -7);
  printf("[%x][%X][%#x][%#X][%#o][%o][%#08x][%.4x][%#.4x]\n", 255, 255, 255, 255, 8, 8, 255, 255, 255);
  printf("[%#x][%#o][%.0d][%.0x][%#.0o][%5.0d]\n", 0, 0, 0, 0, 0, 0);
  printf("[%u][%lu][%llu][%ld][%lld][%hd][%hhd][%hu][%hhu]\n", 4000000000u, 4000000000ul, 18446744073709551615ull, -5l, -9000000000ll, 70000, 200, 70000, 300);
  printf("[%s][%10s][%-10s][%.3s][%10.3s][%-10.3s]\n", name, name, name, name, name, name);
  printf("[%.*s][%*s][%-*s][%*.*s]\n", n, name, w, name, w, name, w, n, name);
  printf("[%.4s][%.*s]\n", raw, 2, raw);
  printf("[%*d][%-*d][%.*d][%*.*d]\n", 5, 42, 5, 42, 4, 42, 8, 4, 42);
  printf("[%*d][%*d]\n", -5, 42, 0, 42);
  printf("[%c][%5c][%-5c]\n", 'a', 'b', 'c');
  printf("[%%][%5%]\n");
  printf("[%+.3d][% 05d][%+05d][%-+5d]\n", 5, 5, 5, 5);
  printf("[%ld][%lx][%#lx]\n", 1234567890123l, 255l, 255l);
  printf("%s %d\n", "end", 1);
  return 0;
}
