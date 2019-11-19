/* ezc/ezc.h - main header file for ezc library

*/


#ifndef EZC_H_
#define EZC_H_

// just include standard headers
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// The stadnard `ezc` integer. I prefer to have larger integers by default,
// And having signed makes everything easier. For way larger values, use zint
//   anyway. (TODO: Use GMP)
typedef int64_t ezc_int;

// The standard `ezc` real number (i.e. floating point). I prefer to have 64bit
//   by default, as this is just much easier. For large values, use the zreal
// type, (TODO: Use MPFR for this)
typedef double  ezc_real;

// The standard `ezc` boolean, i.e. true or false
typedef bool    ezc_bool;

// The 'string' class for ezc, which is NULL-terminated & length-encoded.
// So, ._ can be used in C functions, but also comparing lengths can be faster
// The best of both worlds.
typedef struct {
    char* _;
    int len;
} ezc_str;

// An object, of arbitrary type. Can be a primitive, or, if the index is 
//   >= EZC_TYPE_CUSTOM, a custom type in a context
typedef struct ezc_obj ezc_obj;

// Logging constants
enum {
    // Logging minute things of low priority, most people don't care
    EZC_LOG_TRACE = 0,
    // Logging debug info, not important to most users
    EZC_LOG_DEBUG,
    // Logging standard info, this can be useful, but is off by default
    EZC_LOG_INFO ,
    // DEFAULT: Logs warnings when importing, running, etc
    EZC_LOG_WARN ,
    // Error, this is always on, and logs when an actual error occurs.
    // Most of the time, this is not recoverable, and the program shuts
    //   down.
    EZC_LOG_ERROR,

    EZC_LOG_N
};

// Constructs an ezc_str as a 'view' onto existing memory.
// Therefore, you should not call 'ezc_str_free()' on this,
// And, any modifications made to the view are made to the underlying memory 
#define EZC_STR_VIEW(_charp, _len) ((ezc_str){ ._ = (char*)(_charp), .len = (int)(_len) })

// Returns a view for a constant string literal (this shouldn't be modified)
#define EZC_STR_CONST(_charp) EZC_STR_VIEW(_charp, strlen(_charp))

// The `NULL`, or blanked string. This can be realloced, freed, etc, without any worries
#define EZC_STR_NULL EZC_STR_VIEW(NULL, 0)

enum ezc_type_type {

    // none/null-type
    EZC_TYPE_NONE = 0,

    // wall-type (|)
    EZC_TYPE_WALL,
    
    // boolean, i.e. true or false
    EZC_TYPE_BOOL,

    // signed integer (i.e. int64_t)
    EZC_TYPE_INT,

    // floating-point real
    EZC_TYPE_REAL,

    // string type
    EZC_TYPE_STR,

    // a block of code, i.e. {...}
    EZC_TYPE_BLOCK,

    // index where custom types start
    EZC_TYPE_CUSTOM

};

// represents a 'type' in EZC, with the 4 member functions: `init`, `free`, `repr`, and `copy`
typedef struct {

    int (*f_init)(ezc_obj*);
    int (*f_free)(ezc_obj*);

    int (*f_repr)(ezc_obj*, ezc_str*);
    int (*f_copy)(ezc_obj*, ezc_obj*);

} ezct;

// constucts a type
#define EZCT(_init, _free, _repr, _copy) ((ezct){ .f_init = _init, .f_free = _free, .f_repr = _repr, .f_copy = _copy })

struct ezc_obj {

    // anonymous union, with different structs for types, only
    //   valid if that type is the current one
    union {

        ezc_int int_s;

        ezc_bool bool_s;

        ezc_real real_s;

        ezc_str str_s;

        // just a pointer to the data, if its a custom type
        void* custom_s;

    };


    // what kind of object is it (should be an index into an array)
    int type;

};

#define EZC_OBJ_NONE ((ezc_obj){ .type = EZC_TYPE_NONE })


// type of an instruction
enum ezci_type {

    // do nothing
    EZCI_NOOP = 0,

    // push an integer value on to the stack
    EZCI_INT,

    // push a boolean value on to the stack
    EZCI_BOOL,
    
    // push a real value on to the stack
    EZCI_REAL,

    // push a string literal on to the stack
    EZCI_STR,

    // execute the last item on the stack (!)
    EZCI_BANG,

    // delete the last item on the stack (`)
    EZCI_DEL,

    // copy the last item on the stack (:)
    EZCI_COPY,

    // swaps the last 2 items (<>)
    EZCI_SWAP,

    // gets the item by backwards index (@)
    // or, by string key into the dictionary
    // i.e. UNDER == PEEK(1)
    EZCI_PEEK,

    // adds a block of instructions to the stack
    EZCI_BLOCK,

    // adds a literal wall (|) to the stack
    EZCI_WALL,

    // operators
    EZCI_ADD, EZCI_SUB, EZCI_MUL, EZCI_DIV, EZCI_MOD, EZCI_POW,

    EZCI_UNDER,

    EZCI_EQ,

    // unary operators
    EZCI_USUB,

    EZCI_N

};

typedef struct ezci ezci;
typedef struct ezcp ezcp;

// ezc instruction meta
typedef struct {
    int line, col, len;
    ezcp* prog;
} ezcim;

#define EZCIM_NONE ((ezcim){ .line = 0, .col = 0, .len = 0, .prog = NULL })

// ezc instruction, for executing on the EZC-virtual machine
struct ezci {
    
    union {
        ezc_int _int;
        ezc_bool _bool;
        ezc_real _real;
        ezc_str _str;

        struct ezci_block {
            int n;
            ezci* children;
        } _block;

        //ezci_block* _block;
        void* _ptr;
    };

    // EZCI_* constants, describing what kind of instruction it is
    int type;

    // metadata about the instruction
    ezcim meta;

};

#define EZCI_NONE ((ezci){ .type = EZCI_NOOP, ._ptr = NULL })

// ezc program
struct ezcp {

    ezc_str src_name;
    ezc_str src;

    // normally should be a block
    ezci body;

};

#define EZCP_NONE ((ezcp){ .src_name = EZC_STR_NULL, .src = EZC_STR_NULL, .body = EZCI_NOOP })


typedef struct {

    // 0 at start, grows with stack
    int n;
    ezc_obj* base;

    // internal maximum
    int __max;

} ezc_stk;

#define EZC_STK_NONE ((ezc_stk){ .n = 0, .base = NULL, .__max = 0 })


typedef struct ezc_ctx ezc_ctx;

// function type for operating on a stack, written in C
typedef int (*ezc_cfunc)(ezc_ctx*);

enum ezc_func_type {
    EZC_FUNC_TYPE_C,
    EZC_FUNC_TYPE_EZC,
    EZC_FUNC_N
};

typedef struct {
    int type;

    union {
        ezc_cfunc _c;
        ezci _ezc;
    };

} ezc_func;

#define EZC_FUNC_C(_cfunc) ((ezc_func){ .type = EZC_FUNC_TYPE_C, ._c = _cfunc })
#define EZC_FUNC_EZC(_block) ((ezc_func){ .type = EZC_FUNC_TYPE_EZC, ._ezc = _block })

struct ezc_ctx {
    
    // the global, main stack
    ezc_stk stk;

    // number of functions in a context
    int funcs_n;
    // list of function names
    ezc_str* func_names;
    // list of functions
    ezc_func* funcs;

    // number of types in a context
    int types_n;
    // list of type names
    ezc_str* type_names;
    // list of types
    ezct* types;

};

#define EZC_CTX_NONE ((ezc_ctx){ .stk = EZC_STK_NONE, .funcs_n = 0, .func_names = NULL, .funcs = NULL, .types_n = 0, .type_names = NULL, .types = NULL })



/* text formatting macros */

// Resets all formatting
#define EC_RST   "\033[0m"
// Sets text to be bold
#define EC_BLD   "\033[1m"
// Sets text to be underline
#define EC_ULN   "\033[4m"
// Sets text to be italicized
#define EC_ITA   "\033[3m"

// Sets text to be white (default)
#define EC_WHT   "\033[37m"
// Sets text to be red
#define EC_RED   "\033[31m"
// Sets text to be green
#define EC_GRN   "\033[32m"
// Sets text to be yellow
#define EC_YLW   "\033[33m"
// Sets text to be blue
#define EC_BLU   "\033[34m"
// Sets text to be magenta
#define EC_MAG   "\033[35m"
// Sets text to be cyan
#define EC_CYN   "\033[36m"


/* memory management */

void* ezc_malloc(size_t b);
void* ezc_realloc(void* p, size_t b);
void  ezc_free(void* p);

/* logging/IO */

// prints message (with \n) to the normal output
void ezc_print(const char* fmt, ...);
// print without newline
void ezc_printr(const char* fmt, ...);
// prints the metadata about an instruction
void ezc_printmeta(ezcim meta);

// the base 'logging' function. But, don't use this, use the ezc_trace, ezc_warn,
//   ... etc macro-functions like you would with printf
void ezc_log(int level, const char *file, int line, const char *fmt, ...);

#define ezc_trace(...) ezc_log(EZC_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define ezc_debug(...) ezc_log(EZC_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define ezc_info(...)  ezc_log(EZC_LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define ezc_warn(...)  ezc_log(EZC_LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define ezc_error(...) ezc_log(EZC_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

/* ezc_str functionality */

void ezc_str_init(ezc_str* str);
void ezc_str_concat(ezc_str* str, ezc_str A, ezc_str B);
void ezc_str_concatc(ezc_str* str, char c);
void ezc_str_from(ezc_str* str, char* charp, int len);
void ezc_str_copy(ezc_str* str, ezc_str from);
void ezc_str_free(ezc_str* str);
int ezc_str_cmp(ezc_str A, ezc_str B);
#define ezc_str_eq(_A, _B) (ezc_str_cmp(_A, _B) == 0)

/* ezc_obj functionality */

void ezc_obj_init(ezc_obj* obj, int type);
void ezc_obj_init_int(ezc_obj* obj, ezc_int val);
void ezc_obj_init_real(ezc_obj* obj, ezc_real val);
void ezc_obj_init_str(ezc_obj* obj, ezc_str val);
void ezc_obj_init_from(ezc_obj* obj, ezc_obj* from);

/* ezc_stk functionality */

void ezc_stk_init(ezc_stk* stk);
void ezc_stk_free(ezc_stk* stk);
int ezc_stk_push(ezc_stk* stk, ezc_obj obj);
ezc_obj ezc_stk_pop(ezc_stk* stk);
ezc_obj ezc_stk_get(ezc_stk* stk, int idx);
ezc_obj ezc_stk_peek(ezc_stk* stk);
ezc_obj ezc_stk_peekn(ezc_stk* stk, int back);
void ezc_stk_swap(ezc_stk* stk, int idx, int jdx);

/* ezc_ctx functionality */

void ezc_ctx_init(ezc_ctx* ctx);
//int ezc_ctx_addfunc(ezc_ctx* ctx, ezc_str name, ezcf func);
int ezc_ctx_addfunc(ezc_ctx* ctx, ezc_str name, ezc_func func);
int ezc_ctx_addtype(ezc_ctx* ctx, ezc_str name, ezct type);

int ezc_ctx_getfunci(ezc_ctx* ctx, ezc_str name);
int ezc_ctx_gettypei(ezc_ctx* ctx, ezc_str name);


void ezc_ctx_free(ezc_ctx* ctx);

// compile into a list of instructions
int ezc_compile(ezcp* ret, ezc_str src_name, ezc_str src);

int ezc_exec(ezc_ctx* ctx, ezcp* prog);

//void ezc_exec(ezc_ctx* ctx, ezci* inst, int N);

#endif /* EZC_H_ */


