/* Random operand pairs through the arith group of 128-bit operators (i128_sweep.inc),
   hashed; the hash has to be the C compiler's. */
#define SEED 17
#define COUNT 1000000
#define SWEEP_ARITH 1
#include "i128_sweep.inc"
