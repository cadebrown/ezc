
#include "ezc-impl.h"

void ezc_ctx_init(ezc_ctx* ctx) {
    *ctx = EZC_CTX_NONE;
}

int ezc_ctx_addfunc(ezc_ctx* ctx, ezc_str name, ezc_func func) {
    ctx->funcs_n++;
    ctx->func_names = ezc_realloc(ctx->func_names, sizeof(ezc_str) * ctx->funcs_n);
    ctx->funcs = ezc_realloc(ctx->funcs, sizeof(ezc_func) * ctx->funcs_n);

    ctx->func_names[ctx->funcs_n - 1] = EZC_STR_NULL;
    ezc_str_from(&ctx->func_names[ctx->funcs_n - 1], name._, name.len);

    ctx->funcs[ctx->funcs_n - 1] = func;

    return ctx->funcs_n - 1;    
}


int ezc_ctx_addtype(ezc_ctx* ctx, ezc_str name, ezct type) {
    ctx->types_n++;
    ctx->type_names = ezc_realloc(ctx->type_names, sizeof(ezc_str) * ctx->types_n);
    ctx->types = ezc_realloc(ctx->types, sizeof(ezct) * ctx->types_n);

    ctx->type_names[ctx->types_n - 1] = EZC_STR_NULL;
    ezc_str_from(&ctx->type_names[ctx->types_n - 1], name._, name.len);
    ctx->types[ctx->types_n - 1] = type;

    return ctx->types_n - 1;    
}

// returns index of function, or -1
int ezc_ctx_getfunci(ezc_ctx* ctx, ezc_str name) {
    int i;
    for (i = 0; i < ctx->funcs_n; ++i) {
        if (ezc_str_eq(name, ctx->func_names[i])) {
            return i;
        }
    }
    return -1;
}

int ezc_ctx_gettypei(ezc_ctx* ctx, ezc_str name) {
    int i;
    for (i = 0; i < ctx->types_n; ++i) {
        if (ezc_str_eq(name, ctx->type_names[i])) {
            return i;
        }
    }
    return -1;
}

void ezc_ctx_free(ezc_ctx* ctx) {
    ezc_stk_free(&ctx->stk);
    ezc_free(ctx->types);
    ezc_free(ctx->type_names);
    ezc_free(ctx->func_names);
    ezc_free(ctx->funcs);
    *ctx = EZC_CTX_NONE;
}



