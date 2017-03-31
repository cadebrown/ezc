#include "ezc.h"
#include "ezc_generic.h"

EZC_STACK_TYPE __log(EZC_STACK_TYPE a, EZC_STACK_TYPE b) {
    EZC_STACK_TYPE ret = 0;
    while (a > 1) {
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

void end(void) {
    long long i;
    for (i = 0; i < ptr-1; ++i) {
       printf("%lld, ", vals[i]);
    }
    if (i < ptr) {
        printf("%lld", vals[i]);
    }
    printf("\n");
}

void reset_val(int idx) {
    vals[idx] = 0;
}

void handle_constant(char val[]) {
    vals[ptr] = strtoll(val, NULL, 10);
    ptr++;
}

void handle_operator(int op) {
    int off = 1;
	switch (op) {
        case '<':
            off = -get_last;
            break;
        case '>':
            off = get_last;
            reset_val(ptr-1);
            break;
		case '+':
			get_recent(1) = get_recent(1) + get_recent(0);
            break;
        case '-':
			get_recent(1) = get_recent(1) - get_recent(0);
            break;
        case '*':
			get_recent(1) = get_recent(1) * get_recent(0);
            break;
        case '/':
			get_recent(1) = get_recent(1) / get_recent(0);
            break;
        case '%':
			get_recent(1) = get_recent(1) % get_recent(0);
            break;
        case '^':
			get_recent(1) =__pow(get_recent(1), get_recent(0));
            break;
        case '!':
            off = 0;
			get_recent(0) =__fact(get_recent(0));
            break;
	}
    move_ahead(-off);
}

void handle_function(char val[]) {
    int off = 0;
    if (STR_EQ(val, "sqrt")) {
        get_recent(off) = __sqrt(get_recent(0));
    } else if (STR_EQ(val, "cbrt")) {
        get_recent(off) = __cbrt(get_recent(0));
    } else if (STR_EQ(val, "log")) {
        get_recent(off) = __log(get_recent(0), 2);
    } else if (STR_EQ(val, "logb")) {
        off = 1;
        get_recent(off) = __log(get_recent(1), get_recent(0));
    }
    move_ahead(-off);
}
