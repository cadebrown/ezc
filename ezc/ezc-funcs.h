// ezc/ezc-funcs.h - defines libezc's functions 
//
// Other programs using EZC should include `ezc.h`, which includes this file
//   internally.
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-19
//

#ifndef EZC_FUNCS_H_
#define EZC_FUNCS_H_

#ifndef EZC_H_
#error You included `ezc-funcs.h` by itself, rather than just including `ezc.h`
#endif

#include <stdlib.h>

/* -*- LIB FUNCTIONS -*- */

/* memory management */

// allocates at least `sz` bytes of memory, and returns the address
void* ezc_malloc(size_t sz);
// takes `ptr` (which can be NULL), and attempts to expand its size to `new_sz`
// If `ptr==NULL`, then its the same as returning `ezc_malloc(new_sz)`
void* ezc_realloc(void* ptr, size_t new_sz);
// Frees `ptr` (which can be NULL), and frees the memory at its location
// If `ptr==NULL`, nothing occurs
void  ezc_free(void* ptr);
// copies `sz` bytes from `src` to `dst`
void  ezc_memcpy(void* dst, void* src, size_t sz);

/* logging/IO */

enum {
    // logging at its most verbose, things printed out:
    //   * allocations, frees
    //   * module load attempts
    EZC_LOG_TRACE = 0,
    // logging is quite verbose, printing out:
    //   * files executed, expressions, basic performance
    //   * module searches
    //   * extended version info
    EZC_LOG_DEBUG,
    // logging is minimal during actual execution, but some things are printed:
    //   * module loads, version/build info
    EZC_LOG_INFO,
    // nothing is logged unless something goes slightly wrong, in cases:
    //   * incompatibility detected
    //   * deprecation
    //   * unexpected result
    // (this is the default setting)
    EZC_LOG_WARN,
    // only fatal errors are logged, such as:
    //   * seg faults
    //   * invalid operations
    // this will always print out, there is no way to ignore errors
    EZC_LOG_ERROR
};

// sets the logging level to one of EZC_LOG_* enums
void ezc_log_set_level(int level);
// returns the current logging level
int ezc_log_get_level();

// prints meta about this instruction
void ezc_printmeta(ezci inst);

// a logging function given a EZC_LOG_* enum, the file:line the logging function was called at
//   and the normal printf args
// NOTE: Use the `ezc_*` macros to log for you, like `ezc_error`, and `ezc_warn`...
void  ezc_log(int level, const char* file, int line, const char* fmt, ...);
// logs at the `EZC_LOG_TRACE` level
#define ezc_trace(...) ezc_log(EZC_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
// logs at the `EZC_LOG_DEBUG` level
#define ezc_debug(...) ezc_log(EZC_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
// logs at the `EZC_LOG_INFO` level
#define ezc_info(...) ezc_log(EZC_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
// logs at the `EZC_LOG_WARN` level
#define ezc_warn(...) ezc_log(EZC_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
// logs at the `EZC_LOG_ERROR` level
#define ezc_error(...) ezc_log(EZC_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)


/* -*- TYPE FUNCTIONS -*- */

/* ezc_str functions */

// copies `len` bytes from charp, and then NULL-terminate
void ezc_str_copy_cp(ezc_str* str, char* charp, int len);
// copies a string to another string
void ezc_str_copy(ezc_str* str, ezc_str from);
// concatenates two strings into another one
void ezc_str_concat(ezc_str* str, ezc_str A, ezc_str B);
// appends an entire string
void ezc_str_append(ezc_str* str, ezc_str A);
// appends a character to the string
void ezc_str_append_c(ezc_str* str, char c);
// frees a string and its resources
void ezc_str_free(ezc_str* str);
// compares two strings, should return strcmp(A._, B._)
int ezc_str_cmp(ezc_str A, ezc_str B);
// whether or not the two strings are equal
#define ezc_str_eq(_A, _B) (ezc_str_cmp((_A), (_B)) == 0)


/* ezc_stk functions */

// frees a stack and its resources
void ezc_stk_free(ezc_stk* stk);
// resizes the array to have at least `new_n` objects
void ezc_stk_resize(ezc_stk* stk, int new_n);
// pushes an object onto the stack, returning its index
int ezc_stk_push(ezc_stk* stk, ezc_obj obj);
// pops off an object from the stack.
// If there are no objects on the stack, then it is undefined behaviour
// Please ensure it can pop before it is popped
ezc_obj ezc_stk_pop(ezc_stk* stk);
// gets the idx'th object on the stack
// 0 <= idx < stk->n, otherwise undefined behaviour
ezc_obj ezc_stk_get(ezc_stk* stk, int idx);
// `peeks` at the top of the stack, without popping off
#define ezc_stk_peek(_stk) ezc_stk_get((_stk), (_stk)->n - 1)
// `peeks` at the top - _off, without popping off
#define ezc_stk_peekn(_stk, _off) ezc_stk_get((_stk), (_stk)->n - 1 - _off)
// swaps the objects at index `idx` and `jdx`
// 0 <= idx < stk->n, 0 <= jdx < stk->n
void ezc_stk_swap(ezc_stk* stk, int idx, int jdx);


/* ezcp functions */
// initializes a program from a source string
void ezcp_init(ezcp* prog, ezc_str src_name, ezc_str src);
// frees a program and all its resources
void ezcp_free(ezcp* prog);

/* ezc_vm functions */

// frees a vm and its resources
void ezc_vm_free(ezc_vm* vm);
// adds a function define to the VM
int ezc_vm_addfunc(ezc_vm* vm, ezc_str name, ezc_func func);
// adds a type definition to the VM
int ezc_vm_addtype(ezc_vm* vm, ezc_str name, ezct type);
// returns the index of the function by a given name
// or, -1 if not found
int ezc_vm_getfunci(ezc_vm* vm, ezc_str name);
// returns the index of the type by a given name
// or, -1 if not found
int ezc_vm_gettypei(ezc_vm* vm, ezc_str name);

// executes a program on a VM
int ezc_vm_exec(ezc_vm* vm, ezcp prog);


/* random utility functions */

// initializes the library. This should be called before anything else
int ezc_init();
// finalizes the library. This should be called at the end of your program
void ezc_finalize();
// returns time since ezc_init() was called (in seconds)
double ezc_time();

// return the prefix for this number of bytes (KB, MB, etc)
const char* ezc_bytesize_name(size_t bytes);
// returns the number of ezc_bytesize_name (like 1KB for 1024)
size_t ezc_bytesize_dig(size_t bytes);


#endif /* EZC_FUNCS_H_ */

