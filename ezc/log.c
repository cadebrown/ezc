#include "ezc-impl.h"

static int _lvl = EZC_LOG_INFO;

static const char* _lvl_names[] = {
    EC_WHT "TRACE",
    EC_WHT "DEBUG",
    EC_WHT "INFO ",
    EC_YLW "WARN ",
    EC_RED "ERROR"
};


int ezc_get_loglvl() {
    return _lvl;
}

void ezc_set_loglvl(int new_lvl) {
    if (new_lvl < EZC_LOG_TRACE) new_lvl = EZC_LOG_TRACE;
    else if (new_lvl > EZC_LOG_ERROR) new_lvl = EZC_LOG_ERROR;
    else {
        _lvl = new_lvl;
    }
}


void ezc_print(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    
    fprintf(stdout, "\n");
}

void ezc_printr(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

void ezc_printmeta(ezcim meta) {
    ezc_print("In %*s, @ Line %d, Col %d:", meta.prog->src_name.len, meta.prog->src_name._, meta.line+1, meta.col+1);

    char* posp = meta.prog->src._;
    int _line = 0;
    while (*posp && _line < meta.line) {
        if (*posp == '\n') {
            _line++;
        }
        posp++;
    }
    char* linestart = posp;
    int i = 0;
    //printf("'%s'\n", lstart);
    // now, at beginning of line
    while (*posp && *posp != '\n') {
        if (i == meta.col) {
            ezc_printr(EC_RED EC_BLD);
        } else if (i >= meta.col + meta.len) {
            ezc_printr(EC_RST);

        }
        i++;
        ezc_printr("%c", *posp++);
    }
    ezc_print(EC_RST EC_BLD EC_RED);
    int j;
    for (j = 0; j < i; j++) {
        if (j == meta.col) {
            ezc_printr("^");
        } else if (j >= meta.col && j <= meta.col-1 + meta.len) {
            ezc_printr("~");
        } else {
            ezc_printr(" ");
        }
    }
    ezc_print(EC_RST);
}





void ezc_log(int level, const char *file, int line, const char* fmt, ...) {
    if (level < _lvl) {
        return;
    }

    fprintf(stdout, EC_BLD "%s" EC_RST ": ", _lvl_names[level]);

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

