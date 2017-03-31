#include "ezc.h"
#include "ezc_generic.h"

const unsigned long hash(const char str[]) {
    unsigned long hash = 5381;  
    int c;

    while ((c = *str++) && (c != 0))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

void move_ahead(int num) {
	ptr = ptr + num;
}
