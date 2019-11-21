#include "ezc-impl.h"


/* text formatting macros */

// Resets all formatting
#define EC_RST   "\033[0m"
// Sets text to be bold
#define EC_BLD   "\033[1m"
// Sets text to be underline
#define EC_ULN   "\033[4m"
// Sets text to be italicized
#define EC_ITA   "\033[3m"

// Sets text to be white (default)
#define EC_WHT   "\033[37m"
// Sets text to be red
#define EC_RED   "\033[31m"
// Sets text to be green
#define EC_GRN   "\033[32m"
// Sets text to be yellow
#define EC_YLW   "\033[33m"
// Sets text to be blue
#define EC_BLU   "\033[34m"
// Sets text to be magenta
#define EC_MAG   "\033[35m"
// Sets text to be cyan
#define EC_CYN   "\033[36m"


// current level 
static int _lvl = EZC_LOG_INFO;

static const char* _lvl_names[] = {
    EC_WHT "TRACE",
    EC_WHT "DEBUG",
    EC_WHT "INFO ",
    EC_YLW "WARN ",
    EC_RED "ERROR"
};


int ezc_log_get_level() {
    return _lvl;
}

void ezc_log_set_level(int new_lvl) {
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

void ezc_printmeta(ezci inst) {
    ezc_print("In %*s, @ Line %d, Col %d:", inst.m_prog->src_name.len, inst.m_prog->src_name._, inst.m_line+1, inst.m_col+1);

    char* posp = inst.m_prog->src._;
    int _line = 0;
    while (*posp && _line < inst.m_line) {
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
        if (i == inst.m_col) {
            ezc_printr(EC_RED EC_BLD);
        } else if (i >= inst.m_col + inst.m_len) {
            ezc_printr(EC_RST);

        }
        i++;
        ezc_printr("%c", *posp++);
    }
    ezc_print(EC_RST EC_BLD EC_RED);
    int j;
    for (j = 0; j < i; j++) {
        if (j == inst.m_col) {
            ezc_printr("^");
        } else if (j >= inst.m_col && j <= inst.m_col-1 + inst.m_len) {
            ezc_printr("~");
        } else {
            ezc_printr(" ");
        }
    }
    ezc_print(EC_RST);

}



void ezc_log(int level, const char *file, int line, const char* fmt, ...) {
    if (level < ezc_log_get_level()) {
        return;
    }

    fprintf(stdout, EC_BLD "%s" EC_RST ": ", _lvl_names[level]);

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);
    fprintf(stdout, "\n");
}

