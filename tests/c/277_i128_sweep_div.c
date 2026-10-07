/* Random operand pairs through the div group of 128-bit operators (i128_sweep.inc),
   hashed; the hash has to be the C compiler's. */
#define SEED 29
#define COUNT 1000000
#define SWEEP_DIV 1
#include "i128_sweep.inc"
