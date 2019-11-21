// ezc/ezc-types.h - defines libezc's types 
//
// Other programs using EZC should include `ezc.h`, which includes this file
//   internally.
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-19
//

#ifndef EZC_TYPES_H_
#define EZC_TYPES_H_

// these should be present on Windows, MacOS, and Linux
#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include <string.h>
#include <stddef.h>

/* forward declarations */

typedef struct ezci ezci;
typedef struct ezcp ezcp;
typedef struct ezc_obj ezc_obj;
typedef struct ezc_vm ezc_vm;

/* constants */

#define EZC_SUCCESS 0

/* numeric/builtin types */

// The stadnard `ezc` integer. I prefer to have larger integers by default,
// And having signed makes everything easier. For way larger values, use zint
//   anyway. (TODO: Use GMP)
typedef int64_t ezc_int;
// number of bits in an `ezc_int`
#define EZC_INT_BITS   (sizeof(ezc_int) * 8)
// the value `0` as an `ezc_int`
#define EZC_INT_ZERO   ((ezc_int)0)
// maximum value that can be stored in an `ezc_int`
#define EZC_INT_MAX    ((ezc_int)INT64_MAX)
// minimum value that can be stored in an `ezc_int`
#define EZC_INT_MIN    ((ezc_int)INT64_MIN)

// The standard `ezc` real number (i.e. floating point). I prefer to have 64bit
//   by default, as this is just much easier. For large values, use the zreal
// type, (TODO: Use MPFR for this)
typedef double  ezc_real;
// number of bits in an `ezc_real`
#define EZC_REAL_BITS  (sizeof(ezc_real) * 8)
// the value `0` as an `ezc_real`
#define EZC_REAL_ZERO  ((ezc_real)0.0)
// maximum value that can be stored in an `ezc_int`
#define EZC_REAL_MAX   ((ezc_real)DBL_MAX)
// minimum value that can be stored in an `ezc_int`
#define EZC_REAL_MIN   ((ezc_real)DBL_MIN)

// The standard `ezc` boolean, i.e. true or false
typedef bool    ezc_bool;
// the value `true` as an `ezc_bool`
#define EZC_BOOL_TRUE  ((ezc_bool)true)
// the value `false` as an `ezc_bool`
#define EZC_BOOL_FALSE ((ezc_bool)false)

// The 'string' class for ezc, which is NULL-terminated & length-encoded.
// So, ._ can be used in C functions, but also comparing lengths can be faster
// The best of both worlds.
typedef struct {
    // pointer to the NULL-terminated data
    char* _;
    // length of the string (not including NULL-terminator)
    int len;
    // the maximum length so that reallocation occurs as infrequently as possible
    int max_len;
} ezc_str;
// an `empty` string, which can be used to initialize a string,
// i.e.: a = EZC_STR_EMPTY; will make `a` a valid ezc_str
#define EZC_STR_EMPTY  ((ezc_str){ ._ = NULL, .len = 0, .max_len = 0 })
// the `NULL` string as an ezc_str
#define EZC_STR_NULL   ((ezc_str){ ._ = NULL, .len = 0, .max_len = 0 })
// constructs a `view` of a char pointer and length
// this is still modifiable, but does not make a copy of the data.
// this should not be `free'd`, or realloced or extended or shortened, but
//   can be used to modify in place, or read from
#define EZC_STR_VIEW(_charp, _len) ((ezc_str){ ._ = (char*)(_charp), .len = (int)(_len), .max_len = (int)(_len) })
// Returns a view for a constant string literal (this shouldn't be modified)
#define EZC_STR_CONST(_charp) EZC_STR_VIEW(_charp, strlen(_charp))

// an unsigned integer representing a hash of some object
typedef uint32_t ezc_hash_t; 

#define EZC_HASH_EMPTY ((ezc_hash_t)0)

// structure representing a type within EZC, so just the 4 type functions required
typedef struct {

    int (*f_init)(ezc_obj*);
    int (*f_free)(ezc_obj*);

    int (*f_repr)(ezc_obj*, ezc_str*);
    int (*f_copy)(ezc_obj*, ezc_obj*);

} ezct;
// construct a type from C functions
#define EZCT(_init, _free, _repr, _copy) ((ezct){ .f_init = _init, .f_free = _free, .f_repr = _repr, .f_copy = _copy })


enum {
    // do nothing; noop
    EZCI_NONE = 0,
    // adds a literal wall to the stack (|)
    EZCI_WALL,
    // push a const integer value on to the stack
    EZCI_INT,
    // push a const boolean value on to the stack
    EZCI_BOOL,
    // push a const real value on to the stack
    EZCI_REAL,
    // push a const string on to the stack
    EZCI_STR,
    // adds a block of instructions to the stack ({...})
    EZCI_BLOCK,

    // execute the last item on the stack (!)
    EZCI_EXEC,
    // delete the last item on the stack (`)
    EZCI_DEL,
    // swaps the last 2 items (<>)
    EZCI_SWAP,
    // copy the last item on the stack (:)
    EZCI_COPY,
    // copies the last item under the top on the stack (_)
    EZCI_UNDER,
    // gets the item on the stack with an index ($),
    // OR if negative index, pop on the object indexed in reverse order,
    // OR get a global variable if the top is a string
    // i.e. 0$ == first item on stack
    // 1$ == second item on stack
    // 1~$ == : == item on top of stack
    // 2~$ == _ == item under top of stack
    EZCI_GET,

    // adds the last two items on the stack (+)
    EZCI_ADD, 
    // subtracts the last two items on the stack (-)
    EZCI_SUB, 
    // multiplies the last two items on the stack (*)
    EZCI_MUL, 
    // divides the last two items on the stack (/)
    EZCI_DIV, 
    // performs modulo on the last two items on the stack (%)
    EZCI_MOD, 
    // performs power on the last two items on the stack (^)
    EZCI_POW,
    // negates the item on the top of the stack
    EZCI_USUB,

    // compares the top two items on the stack, then pops on the boolean
    EZCI_EQ,

    // just the number of instruction types
    EZCI_N
};

/* comiler/virtual machine types */

// a structure describing a single `instruction` on the EZC virtual machine
struct ezci {

    union {
        // the literal integer value, only valid if type==EZCI_INT
        ezc_int _int;
        // the literal real value, only valid if type==EZCI_REAL
        ezc_real _real;
        // the literal real value, only valid if type==EZCI_STR
        ezc_str _str;

        // the sub-instructions as part of a block, only valid if
        //   type==EZCI_BLOCK
        struct {
            // number of children in the block
            int n;
            // array of the children
            ezci* children;
            
        } _block;
    };

    // a part of the enum EZCI_*, describing which kind of operation it is
    uint16_t type;

    // meta-data about the parsing of the instruction
    uint16_t m_line, m_col, m_len;

    // meta-data containing a pointer to the program which it was compiled 
    //   under
    ezcp* m_prog;

};
// the empty instruction
#define EZCI_EMPTY ((ezci){ .type = EZCI_NONE, .m_line = 0, .m_col = 0, .m_len = 0, .m_prog = NULL })

// a structure describing a single `program` within EZC
struct ezcp {

    // string containing a 'human-readable' representation of the source
    // i.e. a file name, `-` for stdin, `-e` for an expression pased through
    //   the commandline interface, etc
    ezc_str src_name;

    // string containing the entire source of the program, which is allocated
    // specifically for `ezcp`
    ezc_str src;
    
    // the body instruction of the program. Normally, this is an instruction
    // of type `block`
    ezci body;

};
// the empty program
#define EZCP_EMPTY ((ezcp){ .src_name = EZC_STR_EMPTY, .src = EZC_STR_EMPTY, .body = EZCI_EMPTY })


/* generic types */

enum {
    // the `none`-type/`null`-type
    EZC_TYPE_NONE = 0,
    // the wall type, which is just a single value: `|`
    EZC_TYPE_WALL,
    // the integer type, which should be 64 bit signed (see ezc_int for info)
    EZC_TYPE_INT,
    // the boolean type, which is true/false (see ezc_bool for info)
    EZC_TYPE_BOOL,
    // the real type, which should be a double (see ezc_real for info)
    EZC_TYPE_REAL,
    // the string type (see ezc_str for info)
    EZC_TYPE_STR,
    // an EZC-instruction as an object (see ezci for info)
    // note: this will, most of the time, be a block `{}`
    EZC_TYPE_BLOCK,

    // this is the first index of the non-primitive types, which 
    //   can be tested against to see if the object is builtin or 
    //   one from an outside extension
    EZC_TYPE_CUSTOM
    
};

// structure representing a generic object in EZC
struct ezc_obj {

    union {
        // the integer value of the object (only valid if type==EZC_TYPE_INT)
        ezc_int _int;
        // the boolean value of the object (only valid if type==EZC_TYPE_BOOL)
        ezc_bool _bool;
        // the real value of the object (only valid if type==EZC_TYPE_REAL)
        ezc_real _real;
        // the string value of the object (only valid if type==EZC_TYPE_STR)
        ezc_str _str;
        // the instruction value of the object (only valid if type==EZCI_TYPE_BLOCK)
        ezci _block;

        // generic pointer, used for custom types
        void* _ptr;
    };

    // the type of object, should be an enum in EZC_TYPE_*
    // OR, an index into the context's array of types that have been registered
    // this is testable, by querying `obj.type >= EZC_TYPE_CUSTOM`
    uint16_t type;

};
// the empty object
#define EZC_OBJ_EMPTY ((ezc_obj){ .type = EZC_TYPE_NONE })

// structure representing a stack of objects
typedef struct {

    // array of objects on the stack
    ezc_obj* base;
    
    // number of objects currently on the stack
    int n;

    // the maximum for `n` the pointer is guaranteed to hold,
    //   so it doesn't need to be reallocated until n > max_n
    int max_n;

} ezc_stk;
// the empty stack
#define EZC_STK_EMPTY ((ezc_stk){ .base = NULL, .n = 0, .max_n = 0 })

// function type for operating on a stack, written in C
typedef int (*ezc_cfunc)(ezc_vm*);

enum {
    // the function type is a native C function with the signature: int ()(ezc_vm*)
    EZC_FUNC_TYPE_C,
    EZC_FUNC_TYPE_EZC,
    EZC_FUNC_N
};

// a structure representing an executable function in EZC
typedef struct {

    union {
        // the C native function (only valid if type==EZC_FUNC_TYPE_C)
        ezc_cfunc _c;

        // the EZC instruction to execute (only valid if type==EZC_FUNC_TYPE_EZC)
        ezci _ezc;
    };

    // the type of the function (one of EZC_FUNC_TYPE_* enum)
    int type;

} ezc_func;

// constructs an ezc_func from a C native function with signature: int ()(ezc_vm*)
#define EZC_FUNC_C(_cfunc) ((ezc_func){ .type = EZC_FUNC_TYPE_C, ._c = _cfunc })
// constructs an ezc_func from an EZC instruction to Returns
#define EZC_FUNC_EZC(_ezcfunc) ((ezc_func){ .type = EZC_FUNC_TYPE_EZC, ._ezc = _ezcfunc })


// structure representing the entire state of the VM at once
struct ezc_vm {

    // the global stack of the VM
    ezc_stk stk;

    // structure representing the types in a VM
    struct {
        // number of types
        int n;
        // keys of the types
        ezc_str* keys;
        // values of the types
        ezct* vals;
    } types;

    // structure representing the functions in a VM
    struct {
        // number of functions
        int n;
        // keys of the functions
        ezc_str* keys;
        // values of the functions
        ezc_func* vals;
    } funcs; 

};
// the empty VM
#define EZC_VM_EMPTY ((ezc_vm){ .stk = EZC_STK_EMPTY, .types = { .n = 0, .keys = NULL, .vals = NULL }, .funcs = { .n = 0, .keys = NULL, .vals = NULL } })


#endif /* EZC_TYPES_H_ */

