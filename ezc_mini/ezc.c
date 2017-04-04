#include "ezc.h"
#include "ezc_generic.h"
#include "helper.h"

EZC_STACK global_stk;
EZC_DICT global_dict;


long long globalstop = 0;

char tmpbuf[10000];

void end(EZC_STACK stk) {
    fprintf(stderr, "final stack:\n");
    dump(stk);
    //free(buf);
    exit (1);
}
void fail(char *reason, EZC_STACK stk, char *code, long long pos) {
    long long i;
    fprintf(stderr, "%s\n", reason);
    fprintf(stderr, "%s\n", code);
    i = 0;
    while (i < pos) {
        fprintf(stderr, " ");
        i++;
    }
    fprintf(stderr, "^\n");
    end(stk);
}

void exec_code(EZC_STACK stk, char *code) {
    if (strlen(code) < 1) {
        printf(":%d:\n", code[0]);
    }
    long long i = 0, len = 0;
    while (i < strlen(code)) {
        SKIP_WHITESPACE(code, i);
        if (IS_SPEC(code[i])) {
            ret_special(tmpbuf, code, &i);
            run_special(stk, tmpbuf);
        } else if (STR_STARTS(code, "..", i)) {
            push_copy(stk, (*stk).ptr);
            i+=2;
        } else if (STR_STARTS(code, ".", i)) {
            push_copy(stk, (*stk).ptr);
            i++;
        } else if (code[i] == '"') {
            i++;

            long long tbufptr = 0;
            while (code[i] != '"') {
                tmpbuf[tbufptr++] = code[i++];
            }
            tmpbuf[tbufptr] = 0;
            i++;
            push_str(stk, tmpbuf);
        } else if (code[i] == '[') {
            char *runbuf = (char *)malloc(1000), *ifbuf, *elsebuf;
            ret_subgroup(runbuf, code, &i);
            long long ifd = 0, elsed = 0, maxiter = 0, ct = 0, qres = 0;
            if (STR_STARTS(code, "if", i)) {
                ifbuf = (char *)malloc(1000);
                i += 2;
                ifd = 1;
                ret_subgroup(ifbuf, code, &i);
                if (STR_STARTS(code, "else", i)) {
                    elsebuf = (char *)malloc(1000);
                    i += 4;
                    elsed = 1;
                    ret_subgroup(elsebuf, code, &i);
                }
            }
            DO_ITER(stk, code, i, ct, maxiter,
                if (ifd == 1) {
                    
                    exec_code(stk, ifbuf);
                    
                    qres = RECENT(stk, EZC_INT, 0);
                    move_ahead(stk, -1);
                    if (qres != 0) {
                        
                        exec_code(stk, runbuf);
                    } else if (elsed) {
                        exec_code(stk, elsebuf);
                    }
                } else {
                    exec_code(stk, runbuf);
                }
            )
            SKIP_WHITESPACE(code, i);
        } else if (IS_DIGIT(code[i]) || (IS_SIGN(code[i]) && IS_DIGIT(code[i+1]))) {
            ret_const(tmpbuf, code, &i);
            move_ahead(stk, 1);
            RECENT(stk, EZC_INT, 0) = strtoll(tmpbuf, NULL, 10);
        } else if (IS_OP(code, i)) {
            ret_operator(tmpbuf, code, &i);
            long long maxiter = 0, ct = 0;
            DO_ITER(stk, code, i, ct, maxiter,
                run_operator(stk, tmpbuf);
            )
        } else if (IS_ALPHA(code[i])) {
            ret_function(tmpbuf, code, &i);
            run_function(stk, tmpbuf);
        } else if (i < strlen(code)) {
            fail("Unexpected character", stk, code, i);
        } else {
            break;
        }
    }
}


int main(int argc, char *argv[]) {
    global_stk = (EZC_STACK)malloc(sizeof(EZC_STACK));
    (*global_stk).ptr = -1;
    exec_code(global_stk, argv[1]);

    end(global_stk);
}

