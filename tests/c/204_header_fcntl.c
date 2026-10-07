/* preprocessor stops: no fcntl.h in c2id/include */
#include <fcntl.h>
#include <stdio.h>
int main(void){ printf("%d\n", O_RDONLY); return 0; }
