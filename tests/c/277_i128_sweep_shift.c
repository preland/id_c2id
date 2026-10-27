/* Random operand pairs through the shift group of 128-bit operators (i128_sweep.inc),
   hashed; the hash has to be the C compiler's. */
#define SEED 23
#define COUNT 1000000
#define SWEEP_SHIFT 1
#include "i128_sweep.inc"
