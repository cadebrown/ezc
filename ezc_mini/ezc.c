#include "ezc.h"
#include "ezc_generic.h"
#include "helper.h"

// The `cc` used is a macro in ezc.h for current char

stack_t stk;

long long ptr = -1, bufptr;

char *input, *buf;

void end(void) {
    fprintf(stderr, "final stack:\n");
    gen_dump();
    free(buf);
    exit (1);
}
void fail(char *code, char *reason, long long pos) {
    long long i;
    fprintf(stderr, "%s\n", reason);
    fprintf(stderr, "%s\n", input);
    i = 0;
    while (i < pos) {
        fprintf(stderr, " ");
        i++;
    }
    fprintf(stderr, "^\n");
    end();
}

void exec_code(char *code, long long start, long long len) {
    long long i = 0, s = 0, l = 0, maxiter = 0, ct = 0;
    char *lbuf = (char *)malloc(1000);
    long long lbufptr = 0;
    i = start;
    while (i < start + len) {
        SKIP_WHITESPACE(code, i);
        if (code[i] == '.') {
            gen_push_dupe();
            i++;
        } else if (code[i] == '"') {
            i++;
            lbufptr = 0;
            while (code[i] != '"') {
                lbuf[lbufptr++] = code[i++];
            }
            lbuf[lbufptr] = 0;
            i++;
            gen_push_str(lbuf);
        } else if (code[i] == '[') {
            gen_ret_subgroup(code, &i, &s, &l);
            if (STR_STARTS(code, "if", i)) {
                long long sc=0, lc=0, qres=0;
                i += 2;
                gen_ret_subgroup(code, &i, &sc, &lc);
                DO_ITER(code, i, ct, maxiter,
                    exec_code(code, sc, lc);
                    qres = RECENT(EZC_INT, 0);
                    move_ahead(-1);
                    if (qres) {
                        exec_code(code, s, l);
                    }
                )
            } else {
                DO_ITER(code, i, ct, maxiter, 
                    exec_code(code, s, l);
                )
            }
            SKIP_WHITESPACE(code, i);
        } else if (IS_DIGIT(code[i]) || (IS_SIGN(code[i]) && IS_DIGIT(code[i+1]))) {
            get_const_str(buf, code, &i);
            move_ahead(1);
            __int_from_str(ptr, buf);
        } else if (IS_OP(code, i)) {
            gen_ret_operator(buf, code, &i);
            gen_operator(buf);
        } else if (IS_SPEC(code[i])) {
            gen_ret_special(buf, code, &i);
            gen_special(buf);
        } else if (IS_ALPHA(code[i])) {
            gen_ret_function(buf, code, &i);
            gen_function(buf);
        } else if (i < start + len) {
            fail(code, "Unexpected character", i);
        }
    }
    free(lbuf);
}


int main(int argc, char *argv[]) {
    input = argv[1];
    buf = (char *)malloc(1000);
    exec_code(input, 0, strlen(input));
    end();
}

