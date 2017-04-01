#include "ezc.h"
#include "ezc_generic.h"


void move_ahead(int num) {
	ptr = ptr + num;
}

void gen_setup(void) { }

void gen_dump(void) { 
    long long i;
    for (i = 0; i < ptr; ++i) {
        print_single(stdout, GET(i), GET_F(i));
        fprintf(stdout, ", ");
    }
    if (i <= ptr) print_single(stdout, GET(i), GET_F(i));
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


void push_dupe() {
    move_ahead(1);
    LAST = RECENT(1);
}
void push_str(char *val) {
    move_ahead(1);
    from_str(&LAST, &LAST_F, val);
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
        LAST_F = EZC_SPECIAL_STOP_FLAGS;
    } else if (STR_EQ(spec, "#")) {
        move_ahead(1);
        //LAST = hashstr(spec);
        LAST_F = EZC_SPECIAL_POINT_FLAGS;
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
        __swap(&RECENT(0), &RECENT(1));
        return;
    }

    if (STR_EQ(op, "$")) {
        numargs = 1;
    } else if (STR_EQ(op, "<<") || STR_EQ(op, ">>")) {
        numargs = 0;
    } else {
        numargs = 2;
    }
    if (MEETS_FLAG(LAST_F, EZC_SPECIAL_STOP_FLAGS)) {
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
        EZC_STACK_TYPE op1 = RECENT(op1mv), op0 = RECENT(op0mv);

        /**/ if (STR_EQ(op, "+")) __add(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, "-")) __sub(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, "*")) __mul(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, "/")) __div(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, "%")) __mod(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, "^")) __pow(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, ">")) __gt(&RECENT(op1mv), op1, op0);
        else if (STR_EQ(op, "<")) __lt(&RECENT(op1mv), op1, op0);

    } else if (numargs == 1) {
        int op0mv = 0;
        while (MEETS_FLAG(RECENT_F(op0mv), EZC_SPECIAL_POINT_FLAGS)) {
            op0mv += 1;
        }
        EZC_STACK_TYPE op0 = RECENT(op0mv);
        /**/ if (STR_EQ(op, "$")) RECENT(op0mv) = GET(op0);
    } else if (numargs == 0) {
        /**/ if (STR_EQ(op, ">>")) move_ahead(1);
        else if (STR_EQ(op, "<<")) move_ahead(-1);
    }


    if (numargs == 0) {

    } else {
        move_ahead(1-numargs);
    }
}
