
#ifndef __EZCMODULE_H__
#define __EZCMODULE_H__

#ifndef MODULE_NAME
#error when including 'ezcmodule.h', define MODULE_NAME
#endif

#include "ezctypes.h"

#define PASTER(x,y) x ## _ ## y
#define EVALUATOR(x,y)  PASTER(x,y)

#define STRINGER(x) #x

#define MODULE_FUNCTION(name) EVALUATOR(MODULE_NAME, name)

#endif

