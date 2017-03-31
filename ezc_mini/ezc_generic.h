#include "ezc.h"

void setup(void);

void end(void);


void handle_operator(int op);

void handle_constant(char val[]);

void handle_function(char val[]);


const unsigned long hash(const char str[]);

void reset_val(int idx);

void move_ahead(int num);


#define get_last vals[ptr-1]

#define get_recent(n) vals[ptr-n-1]

#define STR_EQ(a, b) (strcmp(a, b) == 0)