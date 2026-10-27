/* refused: struct bitfield members (width ignored would be wrong) */
#include <stdio.h>
struct Attr { unsigned short aligned : 5, packed : 1, weak : 1; };
int main(void){ struct Attr a; a.aligned = 33; a.packed = 1; a.weak = 0; printf("%d %d %d\n", a.aligned, a.packed, a.weak); return 0; }
