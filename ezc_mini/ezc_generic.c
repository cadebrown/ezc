#include "ezc.h"
#include "ezc_generic.h"

void setup(void) { }

void print_single(EZC_STACK stk, EZC_IDX pos) {
    if (MEETS_FLAG(GET_F(stk, pos), FLAG_STOP)) { 
        printf("| (stop)"); 
    } else if (IS_TYPE(GET_T(stk, pos), TYPE_STR)) { 
        printf("'%s'", GET(stk, char *, pos)); 
    } else if (IS_TYPE(GET_T(stk, pos), TYPE_INT)) {
        __int_print(stk, pos);
    }   else {
        __int_print(stk, pos);
    }

}

void dump(EZC_STACK stk) { 
    long long i;
    for (i = 0; i <= (*stk).ptr; ++i) {
        print_single(stk, i);
        fprintf(stdout, ", ");
    }
    printf("\n");
}



void push_copy(EZC_STACK stk, EZC_IDX pos) {
    move_ahead(stk, 1);
    RECENT(stk, EZC_INT, 0) = GET(stk, EZC_INT, pos);
    RECENT_F(stk, 0) = GET_F(stk, pos);
    RECENT_T(stk, 0) = GET_T(stk, pos);
}

void push_int(EZC_STACK stk, EZC_INT val) {
    __int_push(stk, val);
}

void push_str(EZC_STACK stk, char *val) {
    int len = strlen(val);
    char *kval = (char *)malloc(len);
    strcpy(kval, val);
    move_ahead(stk, 1);
    // IF ERROR, try type of void *
    RECENT(stk, char *, 0) = kval;
    RECENT_F(stk, 0) = FLAG_DEFAULT;
    RECENT_T(stk, 0) = TYPE_STR;
}



EZC_INT pop_int(EZC_STACK stk) {
    EZC_INT ret = RECENT(stk, EZC_INT, 0);
    move_ahead(stk, -1);
    return ret;
}


void swap(EZC_STACK stk, EZC_IDX pos0, EZC_IDX pos1) {
    void *tmp = GET(stk, void *, pos0);
    EZC_TYPE tmpf = GET_F(stk, pos0), tmpt = GET_T(stk, pos0);

    GET(stk, void *, pos0) = GET(stk, void *, pos1);
    GET_F(stk, pos0) = GET_F(stk, pos1);
    GET_T(stk, pos0) = GET_T(stk, pos1);

    GET(stk, void *, pos1) = tmp;
    GET_F(stk, pos1) = tmpf;
    GET_T(stk, pos1) = tmpt;

}


void ret_subgroup(char *out, char *val, EZC_IDX *idx) {
    long long start = 0;

    if (val[*idx] == '[') {
        (*idx)++;
    }

    long long parens = 0;

    while (parens > -1 && !(parens == 0 && (STR_STARTS(val, "if", (*idx)) || STR_STARTS(val, "else", (*idx))))) { 
        if (val[*idx] == ']') {
            parens--;
        } else if (val[*idx] == '[') {
            parens++;
        }
        if (parens > -1) {
            out[start++] = val[(*idx)++];
        } else {
            break;
        }
    }
    
    if (parens == 0 && (STR_STARTS(val, "if", (*idx)) || STR_STARTS(val, "else", (*idx)))) {
    } else {
        (*idx)++;
    }
    out[start] = 0;

}

void ret_ll(char *val, long long *idx, long long *out) {
    (*out) = 0;
    while (IS_DIGIT(val[*idx])) {
        (*out) = 10*(*out) + (val[*idx] - '0');
        (*idx)++;
    }
}


void ret_const(char *out, char *code, long long *start) {
    long long i = (*start);
    if (IS_SIGN(code[i])) {
        out[i-(*start)] = code[i];
        i++;
    }
    while (IS_DIGIT(out[i-(*start)] = code[i])) i++;
    out[i] = 0;
    (*start) = i;
}

void ret_function(char *out, char *code, long long *start) {
    long long i = (*start);
    while (IS_ALPHA(code[i])) {
        out[i-(*start)] = code[i];
        i++;
    }
    out[i-(*start)] = 0;
    (*start) = i;
}

void ret_operator(char *out, char *val, long long *start) {
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

void ret_special(char *out, char *val, long long *start) {
    long long i = 0;
    if (IS_SPEC(val[(*start)])) {
        out[i++] = val[(*start)];
        (*start)++;
    }
    out[i] = 0;
}


void run_function(EZC_STACK stk, char *func) {
    if (STR_EQ(func, "dump")) {
        dump(stk);
    } else if (STR_EQ(func, "print") || STR_EQ(func, "p")) {
        print_single(stk, (*stk).ptr);
        printf("\n");
    } else {
        __int_function(stk, func);
    }
}

void run_operator(EZC_STACK stk, char *op) {
    if (STR_EQ(op, ":")) {
        //EZC_TYPE t0 = RECENT_T(0), t1 = RECENT_T(1);
        swap(stk, (*stk).ptr, (*stk).ptr-1);
    } else {
        __int_op(stk, op);
    }
}

void run_special(EZC_STACK stk, char *spec) {
    if (STR_EQ(spec, "|")) {
        move_ahead(stk, 1);
        RECENT_F(stk, 0) = FLAG_STOP;
        RECENT_T(stk, 0) = TYPE_NIL;
    } else if (STR_EQ(spec, "#")) {
        move_ahead(stk, 1);
        RECENT_F(stk, 0) = FLAG_POINT;
        RECENT_T(stk, 0) = TYPE_NIL;
    }
}












