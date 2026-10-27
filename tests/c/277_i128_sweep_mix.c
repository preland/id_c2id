/* Random operand pairs through the mix group of 128-bit operators (i128_sweep.inc),
   hashed; the hash has to be the C compiler's. */
#define SEED 31
#define COUNT 1000000
#define SWEEP_MIX 1
#include "i128_sweep.inc"
