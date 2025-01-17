#include "spj_stdint.h"
#include "tdd_common.h"
#include <stdlib.h>

PUBLIC U8  randU8(void)  { return (U8)((float)rand() * MAX_U8 / RAND_MAX); }
PUBLIC U16 randU16(void) { return (U16)((float)rand() * MAX_U16 / RAND_MAX); }
PUBLIC U32 randU32(void) { return (U32)((float)rand() * MAX_U32 / RAND_MAX); }
PUBLIC U64 randU64(void) { return (U64)((float)rand() * MAX_U64 / RAND_MAX); }

PUBLIC S8  randS8(void)  { return (S8) ( 2.0 * (float)((S32)rand() - (RAND_MAX/2)) * MAX_S8  / RAND_MAX); }
PUBLIC S16 randS16(void) { return (S16)( 2.0 * (float)((S32)rand() - (RAND_MAX/2)) * MAX_S16 / RAND_MAX); }
PUBLIC S32 randS32(void) { return (S32)( 2.0 * (float)((S32)rand() - (RAND_MAX/2)) * MAX_S32 / RAND_MAX); }
PUBLIC S64 randS64(void) { return (S64)( 2.0 * ((float)rand() - (RAND_MAX/2)) * MAX_S64 / RAND_MAX); }



PUBLIC void randFill(void *dest, size_t numBytes) {
   for(size_t i = 0; i < numBytes; i++, dest++) {
      *((U8*)dest) = randU8();
   }
}

// ------------------------------------------ EOF -------------------------------------------------------
