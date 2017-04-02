#include "ezc.h"
#include "ezc_generic.h"
#include "helper.h"

// The `cc` used is a macro in ezc.h for current char

stack_t stk;

long long ptr = -1, bufptr = 0, globalstop = 0;

char *input, *buf;

void end(void) {
    fprintf(stderr, "final stack:\n");
    gen_dump();
    free(buf);
    exit (1);
}
void fail(char *reason, char *code, long long pos, long long subpos) {
    long long i;
    fprintf(stderr, "%s\n", reason);
    fprintf(stderr, "%s\n", input);
    i = 0;
    while (i < pos) {
        fprintf(stderr, " ");
        i++;
    }
    fprintf(stderr, "^\n");
    fprintf(stderr, "While executing: \n");
    fprintf(stderr, "%s\n", code);
    i = 0;
    while (i < subpos) {
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
        if (IS_SPEC(code[i])) {
            gen_ret_special(buf, code, &i);
            gen_special(buf);
        } else if (STR_STARTS(code, "..", i)) {
            gen_push_copy(ptr-1);
            i+=2;
        } else if (STR_STARTS(code, ".", i)) {
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
            long long ifd = 0, ifs = 0, ifl = 0, qres = 0;
            long long elsed = 0, elses = 0, elsel = 0;
            long long brk = 0;
            if (STR_STARTS(code, "if", i)) {
                i += 2;
                ifd = 1;
                gen_ret_subgroup(code, &i, &ifs, &ifl);
                if (STR_STARTS(code, "else", i)) {
                    i += 4;
                    elsed = 1;
                    gen_ret_subgroup(code, &i, &elses, &elsel);
                }
            }
            DO_ITER(code, i, ct, maxiter,
                if (ifd == 1) {
                    exec_code(code, ifs, ifl);
                    qres = RECENT(EZC_INT, 0);
                    if (qres) {
                        exec_code(code, s, l);
                    } else if (elsed) {
                        exec_code(code, elses, elsel);
                    }
                } else {
                    exec_code(code, s, l);
                }
            )
            SKIP_WHITESPACE(code, i);
        } else if (IS_DIGIT(code[i]) || (IS_SIGN(code[i]) && IS_DIGIT(code[i+1]))) {
            get_const_str(buf, code, &i);
            move_ahead(1);
            __int_from_str(ptr, buf);
        } else if (IS_OP(code, i)) {
            gen_ret_operator(lbuf, code, &i);
            DO_ITER(code, i, ct, maxiter,
                gen_operator(lbuf);
            )
        } else if (IS_ALPHA(code[i])) {
            gen_ret_function(buf, code, &i);
            gen_function(buf);
        } else if (i < start + len) {
            lbufptr = 0;
            while (lbufptr < len) {
                lbuf[lbufptr] = code[lbufptr+start];
                lbufptr++;
            }
            lbuf[lbufptr] = 0;
            fail("Unexpected character", lbuf, i, i - start);
        } else {
            i++;
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

