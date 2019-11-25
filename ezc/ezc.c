// ezc/ezc.c - miscellaneous EZC functions
//
// @author   : Cade Brown <cade@chemicaldevelopment.us>
// @license  : WTFPL (http://www.wtfpl.net/)
// @date     : 2019-11-20
//

#include "ezc-impl.h"
#include <time.h>
#include <sys/time.h>

// counts ezc_init()'s - ezc_finalize()'s
static int g_init_ct = 0;
static struct timeval g_stime = (struct timeval){ .tv_sec = 0, .tv_usec = 0 };

// sizes of 1024 bytes
static const char* g_sizestr[] = {
    "B ",
    "KB",
    "MB",
    "GB",
    "TB",
    "PB",
    "EB"
};

int ezc_init() {
    ezc_debug("ezc_init() called");
    gettimeofday(&g_stime, NULL);
    atexit(ezc_finalize);
    g_init_ct++;
    return EZC_SUCCESS;
}

void ezc_finalize() {
    if (g_init_ct <= 0) {
        //ezc_error("ezc_finalize() called before ezc_init()");
        return;
    }
    ezc_debug("ezc_finalize() called");
    g_init_ct--;
}

double ezc_time() {
    struct timeval curtime;
    gettimeofday(&curtime, NULL);
    return (curtime.tv_sec - g_stime.tv_sec) + 1.0e-6 * (curtime.tv_usec - g_stime.tv_usec);
}

const char* ezc_bytesize_name(size_t bytes) {
    int i = 0;
    while (bytes >= 1024 && i < sizeof(g_sizestr) / sizeof(g_sizestr[0])) {
        i++;
        bytes /= 1024;
    }
    return g_sizestr[i];

}

size_t ezc_bytesize_dig(size_t bytes) {
    int i = 0;
    while (bytes >= 1024 && i < sizeof(g_sizestr) / sizeof(g_sizestr[0])) {
        i++;
        bytes /= 1024;
    }
    return bytes;
}



