// ezc-module.h - the header file for dealing with modules
#ifndef EZC_MODULE_H_
#define EZC_MODULE_H_

#include "ezc.h"

//#define EZC_MODULE_NAME std

// if this is for a specific module
#ifdef EZC_MODULE_NAME

// prefix

// infix between
#define _INFIX(a, b, c) a##b##c 
#define INFIX(a, b, c) _INFIX(a, b, c)

#define EZC_MPFX INFIX(_, EZC_MODULE_NAME, _)

// the signaure for a type-function of 'init'
#define EZC_TF_INIT(_type) static int T##EZC_MPFX##_type##_init(ezc_obj* obj)
// the signaure for a type-function of 'free'
#define EZC_TF_FREE(_type) static int T##EZC_MPFX##_type##_free(ezc_obj* obj)
// the signaure for a type-function of 'repr'
#define EZC_TF_REPR(_type) static int T##EZC_MPFX##_type##_repr(ezc_obj* obj, ezc_str* str)
// the signaure for a type-function of 'copy'
#define EZC_TF_COPY(_type) static int T##EZC_MPFX##_type##_copy(ezc_obj* obj, ezc_obj* from)

// function name
#define EZC_FUNC_NAME(_name) INFIX(F, EZC_MPFX, _name)

// the ezc function of a given name
#define EZC_FUNC(_name) static int EZC_FUNC_NAME(_name)(ezc_ctx* ctx)

// register a type with a context
#define EZC_REGISTER_TYPE(_ctxp, _type) ezc_ctx_addtype(_ctxp, EZC_STR_CONST(#_type), EZCT( \
    T##EZC_MPFX##_type##_init, \
    T##EZC_MPFX##_type##_free, \
    T##EZC_MPFX##_type##_repr, \
    T##EZC_MPFX##_type##_copy  \
));

// register a function by name
#define EZC_REGISTER_FUNC(_ctxp, _name) ezc_ctx_addfunc(_ctxp, EZC_STR_CONST(#_name), EZC_FUNC_C(EZC_FUNC_NAME(_name)));

// forward declaration of registering the module, for `std`, it should be:
//   static int Fstd_register_module(ezc_ctx* ctx);
int EZC_FUNC_NAME(register_module)(ezc_ctx* ctx);

#endif /* EZC_MODULE_NAME */

// these are for all types of source codes

// utility macros for object manipulation
// returns the type of an object
/*#define EZC_T(_obj) (ctx->types[_obj.type])
#define EZC_OBJ_INIT(_obj) (EZC_T(_obj).f_init(&_obj))
#define EZC_OBJ_FREE(_obj) (EZC_T(_obj).f_free(&_obj))
#define EZC_OBJ_COPY(_obj, _from) (EZC_T(_obj).f_copy(&_obj, &_from))
#define EZC_OBJ_REPR(_obj, _str) (EZC_T(obj).f_repr(&_obj, &_str))
*/
//#define EZC_OBJ_TRUTHY(_obj, _val) { if (_obj.type == EZC_TYPE_INT) { _val = _obj.int_s != 0; } else if (_obj.type == EZC_TYPE_STR) { _val = _obj.str_s.len > 0; } else { _val = false; } }

//int ezc_module_register(ezc_ctx* ctx);

#endif /* EZC_MODULE_H_ */

