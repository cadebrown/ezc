#ifndef __EZC_GENERIC_H__
#define __EZC_GENERIC_H__

#include "ezc.h"

#define EZC_CONST_FLAGS           (0x0001)

#define EZC_SPECIAL_FLAGS         (0x0100)
#define EZC_SPECIAL_STOP_FLAGS    (0x0001 | EZC_SPECIAL_FLAGS)
#define EZC_SPECIAL_POINT_FLAGS   (0x0002 | EZC_SPECIAL_FLAGS)

#define MEETS_FLAG(v, f)          (v == f)
#define MEETS_GENFLAG(v, f)       ((v & f) == f)

#ifndef MAXSTACKSIZE
 #define MAXSTACKSIZE (100)
#endif

#ifdef USE_GMP
 #include <gmp.h>
 #define EZC_STACK_TYPE mpz_t
 #define EZC_FLAG_TYPE long long
#else
 #define EZC_STACK_TYPE long long
 #define EZC_FLAG_TYPE long long
#endif

void setup(void);

void dump(void);

void end(void);



void handle_operator(char op);
void handle_special(char op);

void handle_control(char control, char val[]);
void handle_control_stack(char control);

EZC_STACK_TYPE convert_str(char val[]);

void handle_constant(char val[]);

void handle_function(char val[]);


const unsigned long hash(const char str[]);

void reset_val(int idx);

void move_ahead(int num);


#define get_recent(n) vals[ptr-n]
#define get_recent_flags(n) flags[ptr-n]

#define get_last vals[ptr]
#define get_last_flags flags[ptr]

#define STR_EQ(a, b) (strcmp(a, b) == 0)

#endif
