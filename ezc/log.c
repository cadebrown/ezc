#include "log.h"

int log_level = LOG_WARN;

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    if (level > log_level) {
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

