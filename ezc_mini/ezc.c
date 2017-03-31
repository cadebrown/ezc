#include "ezc.h"
#include "ezc_generic.h"

// The `cc` used is a macro in ezc.h for current char
EZC_STACK_TYPE vals[MAXSTACKSIZE];
EZC_FLAG_TYPE flags[MAXSTACKSIZE];

long long ptr = -1, inptr = 0;

char *input, *buf0, *buf1;

void skip_whitespace(void) {
    while (ISSPACE(cc)) {
        inptr++;
    }
    if (cc == 0) {
        terminate_main();
    }
}

void terminate_main(void) {
    fprintf(stderr, "quitting\n");
    end();    
    free(buf0);
    free(buf1);
    exit (1);
}
void fail(char reason[]) {
    long long i;
    fprintf(stderr, "%s\n", reason);
    fprintf(stderr, "%s\n", input);
    i = 0;
    while (i < inptr) {
        fprintf(stderr, " ");
        i++;
    }
    fprintf(stderr, "^\n");
    terminate_main();
}


int main(int argc, char *argv[]) {
    input = argv[1];
    buf0 = (char *)malloc(1000), buf1 = (char *)malloc(1000);
    long long buf0ptr, buf1ptr;

    for (inptr = 0; inptr < strlen(input); ) {
        skip_whitespace();

        if (ISSPECIAL(cc)) {
            if (ISCONTROL(input[inptr+1])) {
                buf0ptr = 0;
                buf0[buf0ptr++] = cc;
                buf0[buf0ptr] = 0;
                inptr++;
                handle_control(cc, buf0);
            } else {
                handle_special(cc);                
            }
            inptr++;
        } else if (ISDIGIT(cc) || (cc == '-' && ISDIGIT(input[inptr+1]))) {
            buf0ptr = 0;
            while (ISDIGIT(cc) || cc == '-') {
                buf0[buf0ptr++] = input[inptr++];
            }
            buf0[buf0ptr] = 0;

            if (ISCONTROL(cc)) {
                handle_control(cc, buf0);
                inptr++;
            } else {
                handle_constant(buf0);
            }
        } else if (ISCONTROL(cc)) {
            handle_control_stack(cc);
            inptr++;
        } else if (ISOP(cc)) {
            if (input[inptr+1] == '&') {
                char op = cc;
                inptr = inptr + 2; // skip over & and op as well
                int cap = 0, ct = 0;
                if (ISDIGIT(cc)) {
                    buf0ptr = 0;
                    while (ISDIGIT(cc)) {
                        buf0[buf0ptr++] = input[inptr++];
                    }
                    buf0[buf0ptr] = 0;
                    cap = convert_str(buf0);
                }
                while ((!MEETS_FLAG(get_recent_flags(1), EZC_SPECIAL_STOP_FLAGS) && ptr > 0) && (cap == 0 || ct < cap)) {
                    handle_operator(op);
                    ct++;
                }
            } else {
                handle_operator(cc);
            }
            inptr++;
        } else if (ISFUNC(cc)) {
            buf0ptr = 0;
            while (ISFUNC(cc)) {
                buf0[buf0ptr++] = input[inptr++];
            }
            buf0[buf0ptr] = 0;
            handle_function(buf0);
        } else {
            fail("Unexpected symbol");
        }
    }
    terminate_main();
}

