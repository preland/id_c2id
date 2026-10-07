/* A million random operand pairs through + - * /, the comparisons and the narrowing
   conversions, hashed (ld_sweep.inc); the hash has to be the C compiler x87's. */
#define SEED 21
#define COUNT 1000000
#include "ld_sweep.inc"
