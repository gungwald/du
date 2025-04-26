/* Switch between real gc and regular C functions */

#ifndef GC_SWITCH_1928342810562
#define GC_SWITCH_1928342810562

#ifdef GC_REAL
#include <gc.h>
#else
#define GC_INIT()
#define GC_MALLOC(x) malloc(x)
#define GC_FREE free(x)
#endif

#endif
