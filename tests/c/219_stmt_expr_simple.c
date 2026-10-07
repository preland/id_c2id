/* SILENT WRONG PROGRAM: GNU statement expression ({ ... }); c2id prints 'expression expected' on stdout, exits 0, project builds and prints nothing, gcc prints 6 */
#include <stdio.h>
int main(void){
  int x = ({ int t = 3; t * 2; });
  printf("%d\n", x);
  return 0;}
