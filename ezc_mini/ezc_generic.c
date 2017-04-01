#include "ezc.h"
#include "ezc_generic.h"

void move_ahead(int num) {
	ptr = ptr + num;
}

void gen_setup(void) { }

void gen_print_single(int pos) {
    if (MEETS_FLAG(GET_F(pos), EZC_STR_FLAGS)) { 
        fprintf(stdout, "'%s'", GET(char *, pos)); 
    } else { 
        print_single(stdout, GET(long long, pos), GET_F(pos)); 
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

void gen_ret_ll(char *val, long long *idx, long long *out) {
    (*out) = 0;
    while (IS_DIGIT(val[*idx])) {
        (*out) = 10*(*out) + (val[*idx] - '0');
        (*idx)++;
    }
}


void gen_push_dupe() {
    move_ahead(1);
    RECENT(long long, 0) = RECENT(long long, 1);
}


void gen_push_str(char *val) {
    int len = strlen(val);
    char *kval = (char *)malloc(len);
    strcpy(kval, val);
    move_ahead(1);
    stk.vals[ptr] = (void *)kval;
    RECENT_F(0) = EZC_STR_FLAGS;
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

void gen_ret_subgroup(char *val, long long *idx, long long *start, long long *len) {
    long long parencount = 0;
    (*len) = 0;
    (*start) = 0;


    if (val[(*idx)] == '[') {
        ++(*idx);
        ++parencount;
    }
    (*start) = (*idx);


    while (!STR_STARTS(val, "if", (*idx))) { 

        if (val[*idx] == ']') {
            parencount--;
        } else if (val[*idx] == '[') {
            parencount++;
        }
        if (parencount == 0) {
            break;
        }
        (*idx)++;
        (*len)++;
    }
    if (val[(*idx)] == ']') {
        (*idx)++;
    }
}

void gen_swap(long long pos0, long long pos1) {
    void *tmp = GET(void *, pos0);
    long long tmpf = GET_F(pos0), tmpt = GET_T(pos0);
    gen_print_single(pos0);
    gen_print_single(pos1);
    printf("\n");
    GET(void *, pos0) = GET(void *, pos1);
    GET_F(pos0) = GET_F(pos1);
    GET_T(pos0) = GET_T(pos1);
    GET(void *, pos1) = tmp;
    GET_F(pos0) = tmpf;
    GET_T(pos0) = tmpt;
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
        //LAST = hashstr(spec);
        RECENT_F(0) = EZC_SPECIAL_STOP_FLAGS;
    } else if (STR_EQ(spec, "#")) {
        move_ahead(1);
        //LAST = hashstr(spec);
        RECENT_F(0) = EZC_SPECIAL_POINT_FLAGS;
    }
}

void gen_ret_operator(char *out, char *val, long long *start) {
    long long i = 0;
    if (STR_STARTS(val, "<<", (*start))) {
        out[i++] = '<';
        out[i++] = '<';
        (*start) += 2;
    } else if (STR_STARTS(val, ">>", (*start))) {
        out[i++] = '>';
        out[i++] = '>';
        (*start) += 2;
    } else if (IS_OP(val, (*start))) {
        out[i++] = val[(*start)];
        (*start)++;
    }
    out[i] = 0;
}

void gen_operator(char *op) {
    int numargs;
    if (STR_EQ(op, ":")) {
        long long t0 = RECENT_T(0), t1 = RECENT_T(1);
        /*if (t0 == t1) {
            
        } else {

        }*/
        gen_swap(ptr, ptr-1);
        //__swap(&GET_DYN(ptr), &GET_DYN(ptr-1));
        return;
    }

    if (STR_EQ(op, "$")) {
        numargs = 1;
    } else if (STR_EQ(op, "<<") || STR_EQ(op, ">>")) {
        numargs = 0;
    } else {
        numargs = 2;
    }
    if (MEETS_FLAG(RECENT_F(0), EZC_SPECIAL_STOP_FLAGS)) {
        return;
    }

    if (numargs == 2) {
        int op0mv = 0;
        while (MEETS_FLAG(RECENT_F(op0mv), EZC_SPECIAL_POINT_FLAGS)) {
            op0mv += 1;
        }
        int op1mv = op0mv+1;
        while (MEETS_FLAG(RECENT_F(op1mv), EZC_SPECIAL_POINT_FLAGS)) {
            op1mv += 1;
        }
        EZC_STACK_TYPE op1 = RECENT(long long, op1mv), op0 = RECENT(long long, op0mv);

        /**/ if (STR_EQ(op, "+")) __add(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, "-")) __sub(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, "*")) __mul(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, "/")) __div(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, "%")) __mod(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, "^")) __pow(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, ">")) __gt(&RECENT(long long, op1mv), op1, op0);
        else if (STR_EQ(op, "<")) __lt(&RECENT(long long, op1mv), op1, op0);

    } else if (numargs == 1) {
        int op0mv = 0;
        while (MEETS_FLAG(RECENT_F(op0mv), EZC_SPECIAL_POINT_FLAGS)) {
            op0mv += 1;
        }
        EZC_STACK_TYPE op0 = RECENT(long long, op0mv);
        /**/ if (STR_EQ(op, "$")) RECENT(long long, op0mv) = GET(long long, op0);
    } else if (numargs == 0) {
        /**/ if (STR_EQ(op, ">>")) move_ahead(1);
        else if (STR_EQ(op, "<<")) move_ahead(-1);
    }


    if (numargs == 0) {

    } else {
        move_ahead(1-numargs);
    }
}
