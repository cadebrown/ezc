
#ifndef __EZC_H__
#define __EZC_H__

#include <stdio.h>
#include <stdlib.h>

#include "ezc_types.h"
#include "obj/obj.h"
#include "stack/stack.h"
#include "dict/dict.h"

void exec(EZC_STR code, EZC_DICT dict, EZC_STACK stk);

int main(int argc, char *argv[]);

#endif
