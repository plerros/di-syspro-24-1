#ifndef SYSPROG24_1_CONFIGURATION_ADV_H
#define SYSPROG24_1_CONFIGURATION_ADV_H

#if SAFETY_CHECKS
#define OPTIONAL_ASSERT(x) assert(x)
#include <assert.h>
#else
#define OPTIONAL_ASSERT(x) if(!(x)) {no_args();}
#endif

#endif /*SYSPROG24_1_CONFIGURATION_ADV_H*/