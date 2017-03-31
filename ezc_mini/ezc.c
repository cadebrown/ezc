#include "ezc.h"
#include "ezc_generic.h"


EZC_STACK_TYPE vals[MAXSTACKSIZE];

long long ptr = 0;

char *input;

int main(int argc, char *argv[]) {
    input = argv[1];
    char *buf = (char *)malloc(1000);
    long long i, bufptr;

    for (i = 0; i < strlen(input); ) {
        while (input[i] == ' ' || input[i] == ',') {
            i++;
        }
        if (ISDIGIT(input[i]) || (input[i] == '-' && ISDIGIT(input[i+1]))) {
            bufptr = 0;
            while (ISDIGIT(input[i]) || input[i] == '-') {
                buf[bufptr++] = input[i++];
            }
            buf[bufptr] = 0;
            handle_constant(buf);
        } else if (ISOP(input[i])) {
            handle_operator(input[i]);
            i++;

        } else if (ISALPHA(input[i])) {
            bufptr = 0;
            while (ISALPHA(input[i])) {
                buf[bufptr++] = input[i++];
            }
            buf[bufptr] = 0;
            handle_function(buf);
        }
    }

    end();
}

