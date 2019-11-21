// ezc/ezc-module.h - the header to include when createing a module in C 
//
// This provides utilities for defining the correct function names, and types, 
//   and provides utility macros for registering types/functions
//
// BEFORE INCLUDING `ezc-module.h`, you should:
//   * include `ezc.h`
//   * define EZC_MODULE_NAME to the name of your module,
//       i.e. #define EZC_MODULE_NAME foobar
//
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-19
//

#ifndef EZC_MODULE_H_
#define EZC_MODULE_H_

// if this is for a specific module
#ifdef EZC_MODULE_NAME

#define _EZCPASTE(a, b, c) a##b##c 
// just concatenates abc (with 1 level of indirection)
#define EZCPASTE(a, b, c) _EZCPASTE(a, b, c)

// the module infix, infixed between T and the type name/func name
// OR, F and the function name
#define EZC_MIFX EZCPASTE(_, EZC_MODULE_NAME, _)

// the semi-mangled function name
#define EZC_FUNC_NAME(_name) EZCPASTE(F, EZC_MIFX, _name)

// the signaure for a type-function of 'init'
#define EZC_TF_INIT(_type) static int T##EZC_MIFX##_type##_init(ezc_obj* obj)
// the signaure for a type-function of 'free'
#define EZC_TF_FREE(_type) static int T##EZC_MIFX##_type##_free(ezc_obj* obj)
// the signaure for a type-function of 'repr'
#define EZC_TF_REPR(_type) static int T##EZC_MIFX##_type##_repr(ezc_obj* obj, ezc_str* str)
// the signaure for a type-function of 'copy'
#define EZC_TF_COPY(_type) static int T##EZC_MIFX##_type##_copy(ezc_obj* obj, ezc_obj* from)

// the ezc function of a given name (defines the parameter as being named `vm`)
#define EZC_FUNC(_name) static int EZC_FUNC_NAME(_name)(ezc_vm* vm)

// register a type with a VM (requires the name of the pointer of the vm to be `vm`)
#define EZC_REGISTER_TYPE(_type) ezc_vm_addtype(vm, EZC_STR_CONST(#_type), EZCT( \
    T##EZC_MIFX##_type##_init, \
    T##EZC_MIFX##_type##_free, \
    T##EZC_MIFX##_type##_repr, \
    T##EZC_MIFX##_type##_copy  \
));

// register a function by name (assumes you've defined it like: EZC_FUNC(name) { ... })
#define EZC_REGISTER_FUNC(_name) ezc_vm_addfunc(vm, EZC_STR_CONST(#_name), EZC_FUNC_C(EZC_FUNC_NAME(_name)));

// forward declaration of registering the module, for `std`, it should be:
//   static int Fstd_register_module(ezc_ctx* ctx);
extern int EZC_FUNC_NAME(register_module)(ezc_vm* vm);


#else /* ifdef EZC_MODULE_NAME */


#error EZC_MODULE_NAME was not defined


#endif /* ifdef EZC_MODULE_NAME */

#endif /* EZC_MODULE_H_ */

