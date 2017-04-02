#include "ezc.h"
#include "ezc_generic.h"

void move_ahead(int num) {
	ptr = ptr + num;
}
EZC_INT gen_pop_int(void) {
    EZC_INT ret = RECENT(EZC_INT, 0);
    move_ahead(-1);
    return ret;
}

void gen_setup(void) { }

void gen_print_single(int pos) {
    if (MEETS_FLAG(GET_F(pos), FLAG_STOP)) { 
        printf("| (stop)"); 
    } else if (IS_TYPE(GET_T(pos), TYPE_STR)) { 
        printf("'%s'", GET(char *, pos)); 
    } else if (IS_TYPE(GET_T(pos), TYPE_INT)) {
        __int_print(pos);
    }  
}

void gen_dump(void) { 
    long long i;
    for (i = 0; i <= ptr; ++i) {
        gen_print_single(i);
        fprintf(stdout, ", ");
    }
    printf("\n");
}

void gen_end(void) {
    gen_dump();
}

void gen_push_dupe() {
    move_ahead(1);
    RECENT(EZC_INT, 0) = RECENT(EZC_INT, 1);
    RECENT_F(0) = RECENT_F(1);
    RECENT_T(0) = RECENT_T(1);
}
void gen_push_copy(EZC_IDX pos) {
    move_ahead(1);
    RECENT(EZC_INT, 0) = GET(EZC_INT, pos);
    RECENT_F(0) = GET_F(pos);
    RECENT_T(0) = GET_T(pos);
}

void gen_push_int(EZC_INT val) {
    __int_push(val);
}

void gen_push_str(char *val) {
    int len = strlen(val);
    char *kval = (char *)malloc(len);
    strcpy(kval, val);
    move_ahead(1);
    stk.vals[ptr] = (void *)kval;
    RECENT_F(0) = FLAG_DEFAULT;
    RECENT_T(0) = TYPE_STR;
}

void get_const_str(char *out, char *code, long long *start) {
    long long i = (*start);
    if (IS_SIGN(code[i])) {
        out[i-(*start)] = code[i];
        i++;
    }
    while (IS_DIGIT(out[i-(*start)] = code[i])) i++;
    out[i] = 0;
    (*start) = i;
}

void gen_ret_function(char *out, char *code, long long *start) {
    long long i = (*start);
    while (IS_ALPHA(code[i])) {
        out[i-(*start)] = code[i];
        i++;
    }
    out[i-(*start)] = 0;
    (*start) = i;
}

void gen_ret_subgroup(char *val, long long *idx, long long *start, long long *len) {
    (*len) = 0;
    (*start) = 0;

    if (val[*idx] == '[') {
        (*idx)++;
    }

    (*start) = (*idx);

    long long parens = 0;
    while (parens > -1 && !(parens == 0 && (STR_STARTS(val, "if", (*idx)) || STR_STARTS(val, "else", (*idx))))) { 

        if (val[*idx] == ']') {
            parens--;
        } else if (val[*idx] == '[') {
            parens++;
        }
        if (parens > -1) {
            (*idx)++;
            (*len)++;
        } else {
            break;
        }
    }
    if (parens == 0 && (STR_STARTS(val, "if", (*idx)) || STR_STARTS(val, "else", (*idx)))) {

    } else {
        (*idx)++;
    }
}

void gen_swap(long long pos0, long long pos1) {
    void *tmp = GET(void *, pos0);
    long long tmpf = GET_F(pos0), tmpt = GET_T(pos0);

    GET(void *, pos0) = GET(void *, pos1);
    GET_F(pos0) = GET_F(pos1);
    GET_T(pos0) = GET_T(pos1);

    GET(void *, pos1) = tmp;
    GET_F(pos1) = tmpf;
    GET_T(pos1) = tmpt;

}

void gen_ret_special(char *out, char *val, long long *start) {
    long long i = 0;
    if (IS_SPEC(val[(*start)])) {
        out[i++] = val[(*start)];
        (*start)++;
    }
    out[i] = 0;
}

void gen_special(char *spec) {
    if (STR_EQ(spec, "|")) {
        move_ahead(1);
        RECENT_F(0) = FLAG_STOP;
        RECENT_T(0) = TYPE_NIL;
    } else if (STR_EQ(spec, "#")) {
        move_ahead(1);
        RECENT_F(0) = FLAG_POINT;
        RECENT_T(0) = TYPE_NIL;
    }
}
void gen_ret_ll(char *val, long long *idx, long long *out) {
    (*out) = 0;
    while (IS_DIGIT(val[*idx])) {
        (*out) = 10*(*out) + (val[*idx] - '0');
        (*idx)++;
    }
}

void gen_ret_operator(char *out, char *val, long long *start) {
    long long i = 0;
    if (STR_STARTS(val, "==", (*start))) {
        out[i++] = '=';
        out[i++] = '=';
    } else if (STR_STARTS(val, "<<", (*start))) {
        out[i++] = '<';
        out[i++] = '<';
    } else if (STR_STARTS(val, ">>", (*start))) {
        out[i++] = '>';
        out[i++] = '>';
    } else if (IS_OP(val, (*start))) {
        out[i++] = val[(*start)];
    }
    out[i] = 0;
    (*start) += strlen(out);
}

void gen_operator(char *op) {
    if (STR_EQ(op, ":")) {
        EZC_TYPE t0 = RECENT_T(0), t1 = RECENT_T(1);
        gen_swap(ptr, ptr-1);
    } else {
        __int_op(op);
    }
}


void gen_function(char *func) {
    if (STR_EQ(func, "dump")) {
        gen_dump();
    } else if (STR_EQ(func, "print") || STR_EQ(func, "p")) {
        gen_print_single(ptr);
        printf("\n");
    } else {
        __int_function(func);
    }
}

