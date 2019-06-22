#ifndef TDD_COMMON_H
#define TDD_COMMON_H

#include "GenericTypeDefs.h"
#include <time.h>
#include <stddef.h>

PUBLIC U8  randU8(void);
PUBLIC U16 randU16(void);
PUBLIC U16 randU32(void);
PUBLIC S16 randS16(void);
PUBLIC S32 randS32(void);
PUBLIC S8  randS8(void);
PUBLIC void randFill(void *dest, size_t numBytes);

#endif // TDD_COMMON_H
