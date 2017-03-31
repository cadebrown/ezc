#include "ezc.h"
#include "ezc_generic.h"

EZC_STACK_TYPE __log(EZC_STACK_TYPE a, EZC_STACK_TYPE b) {
    EZC_STACK_TYPE ret = 0;
    while (a >= b) {
        a /= b;
        ret++;
    }
    return ret;
}

EZC_STACK_TYPE __sqrt(EZC_STACK_TYPE a) {
    EZC_STACK_TYPE ret = a >> 1, lret = -1;
    while ((ret = ret/2 + a/(2 * ret)) != lret && ret != lret + 1) {
        lret = ret;
    }
    return (ret * ret > a) ? lret : ret;
}

EZC_STACK_TYPE __cbrt(EZC_STACK_TYPE a) {
    if (a < 0) {
        return -__cbrt(-a);
    }
    EZC_STACK_TYPE ret = 1+(a >> 1), lret = -1;
    while ((ret = (ret + a / (ret * ret))/2) != lret && ret != lret + 1) {
        lret = ret;
    }
    return (ret * ret * ret > a) ? lret : ret;
}


EZC_STACK_TYPE __pow(EZC_STACK_TYPE a, EZC_STACK_TYPE b) {
    if (b == 0) {
        return 1;
    } else if (b == 1) {
        return a;
    } else if (b == 2) {
        return a * a;
    }
    if (a == 0 || a == 1) {
        return a;
    }
    return ((b & 1)*(a-1)+1) * __pow(a*a, b>>1);
}


EZC_STACK_TYPE __fact(EZC_STACK_TYPE a) {
    EZC_STACK_TYPE i, ret = 1;
    i = 1;
    while (i <= a) {
        ret = ret * i;
        i++;
    }
    return ret;
}

void setup(void) { }

void print_single(long long i) {
    if (MEETS_GENFLAG(flags[i], EZC_SPECIAL_FLAGS)) {
        printf("%c", (char)vals[i]);
    } else {
        printf("%lld", vals[i]);
    }
}

void dump(void) {
    long long i;
    for (i = 0; i < ptr; ++i) {
        print_single(i);
        printf(", ");
    }
    if (i <= ptr) print_single(i);
    printf("\n");
}

void end(void) {
    dump();
}

void reset_val(int idx) {
    vals[idx] = 0;
    flags[idx] = EZC_CONST_FLAGS;
}

EZC_STACK_TYPE convert_str(char val[]) {
    return (EZC_STACK_TYPE) strtoll(val, NULL, 10);
}

void handle_constant(char val[]) {
    ptr++;
    vals[ptr] = convert_str(val);
    flags[ptr] = EZC_CONST_FLAGS;
}

void handle_control(char control, char val[]) {
    EZC_FLAG_TYPE srch, vv = convert_str(val);
    int ssw = 0;
    if (ISSPECIAL(val[0])) {
        ssw = 1;
        if (val[0] == '#') {
            srch = EZC_SPECIAL_POINT_FLAGS;
        } else if (val[0] == '|') {
            srch = EZC_SPECIAL_STOP_FLAGS;
        }
    }
	switch (control) {
		case '<':
        
            if (ssw == 1) {
                while (!MEETS_FLAG(get_last_flags, srch) && ptr > 0) {
                    move_ahead(-1);
                }
                move_ahead(-1);
            } else {
			    move_ahead(-vv);
            }
            break;
		case '>':

            if (ssw == 1) {
                while (!MEETS_FLAG(get_last_flags, srch)) {
                    move_ahead(1);
                }
                move_ahead(-1);
            } else {
			    move_ahead(vv);
            }
    }
}

void handle_control_stack(char control) {
    EZC_FLAG_TYPE srch;
    int ssw = 0;
    if (MEETS_GENFLAG(get_last_flags, EZC_SPECIAL_FLAGS)) {
        ssw = 1;
        if (MEETS_FLAG(get_last_flags, EZC_SPECIAL_POINT_FLAGS)) {
            srch = EZC_SPECIAL_POINT_FLAGS;
        } else if (MEETS_FLAG(get_last_flags, EZC_SPECIAL_STOP_FLAGS)) {
            srch = EZC_SPECIAL_STOP_FLAGS;
        }
    }
	switch (control) {
		case '<':
            if (ssw == 1) {
                while (!MEETS_FLAG(get_last_flags, srch) && ptr > 0) {
                    move_ahead(-1);
                }
                move_ahead(-1);
            } else {
			    move_ahead(-get_last);
            }
            break;
		case '>':
            if (ssw == 1) {
                while (!MEETS_FLAG(get_last_flags, srch)) {
                    move_ahead(1);
                }
                move_ahead(-1);
            } else {
			    move_ahead(get_last);
            }
            break;
    }
}
void handle_special(char spec) {
	switch (spec) {
		case '|':
			ptr++;
            vals[ptr] = spec;
            flags[ptr] = EZC_SPECIAL_STOP_FLAGS;
            break;
		case '#':
			ptr++;
            vals[ptr] = spec;
            flags[ptr] = EZC_SPECIAL_POINT_FLAGS;
            break;
    }
}

void handle_operator(char op) {
    int mv = -1;
    if (MEETS_FLAG(get_recent_flags(0), EZC_SPECIAL_STOP_FLAGS)) {
        return;
    }
    int op0mv = 0;
    while (MEETS_FLAG(get_recent_flags(op0mv), EZC_SPECIAL_POINT_FLAGS)) {
        op0mv += 1;
    }
    int op1mv = op0mv+1;
    while (MEETS_FLAG(get_recent_flags(op1mv), EZC_SPECIAL_POINT_FLAGS)) {
        op1mv += 1;
    }

    EZC_STACK_TYPE op1 = get_recent(op1mv), op0 = get_recent(op0mv);

	switch (op) {
		case '+':
			get_recent(op1mv) = op1 + op0;
            break;
        case '-':
			get_recent(op1mv) = op1 - op0;
            break;
        case '*':
			get_recent(op1mv) = op1 * op0;
            break;
        case '/':
			get_recent(op1mv) = op1 / op0;
            break;
        case '%':
			get_recent(op1mv) = op1 % op0;
            break;
        case '^':
			get_recent(op1mv) =__pow(op1, op0);
            break;
        case '!':
            mv = 0;
			get_recent(op0mv) =__fact(op0);
            break;
	}
    if (mv == 0) {
    } else if (mv == -1) {
        //get_recent(op1mv) = 0;
        get_recent_flags(op1mv) = EZC_CONST_FLAGS;
        move_ahead(-op1mv);

    } else {
        fprintf(stderr, "ERROR: Not prepared!");
    }
}

void handle_function(char val[]) {
    int mv = 0;
    if (STR_EQ(val, "sqrt")) {
        get_recent(0) = __sqrt(get_recent(0));
    } else if (STR_EQ(val, "cbrt")) {
        get_recent(0) = __cbrt(get_recent(0));
    } else if (STR_EQ(val, "log")) {
        get_recent(0) = __log(get_recent(0), 2);
    } else if (STR_EQ(val, "logb")) {
        mv = -1;
        get_recent(1) = __log(get_recent(0), get_recent(1));
    } else if (STR_EQ(val, ":")) {
        //mv = -1;
        EZC_STACK_TYPE tmp = get_recent(0);
        EZC_FLAG_TYPE tmpf = get_recent_flags(0);
        get_recent(0) = get_recent(1);
        get_recent_flags(0) = get_recent_flags(1);
        get_recent(1) = tmp;
        get_recent_flags(1) = tmpf;
    }
    move_ahead(mv);
}
