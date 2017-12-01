#include "log.h"


static struct {
    void *udata;
    FILE *fp;
    int level;
    int quiet;
} L;

void log_set_udata(void *udata) {
    L.udata = udata;
}

void log_set_fp(FILE *fp) {
    L.fp = fp;
}

int log_get_level() {
    return L.level;
}

bool log_isinit = false;
void log_init() {
    if (log_isinit) {
        return;
    } else {
        log_set_level(LOG_WARN);
    }

}


void log_set_level(int level) {
    L.level = level;
}


void log_set_quiet(int enable) {
    L.quiet = enable ? 1 : 0;
}


void log_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level > L.level) {
        return;
    }

    /*
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);

    char buf[16];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';

    printf("%s %-5s (T%d) %s:%d: ", buf, level_names[level], world_rank, file, line);

    */

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
}

