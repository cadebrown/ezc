#ifndef __EZC_TYPES_H__
#define __EZC_TYPES_H__

#ifndef USEGMP
 #define EZC_MP mpz_t
#else
 #define EZC_MP err
#endif

#define EZC_INT long long
#define EZC_STR char *

#define EZC_FLAG long long
#define EZC_TYPE long long
#define EZC_IDX long long


#endif
