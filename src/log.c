#include <stdio.h>
#include "log.h"

log_level_t log_level = LOG_INFO;

void print_log(log_level_t level, const char * fmt, ...) {
    if ( level >= log_level ) {
        switch ( level ) {
            case LOG_DEBUG:
                fprintf(stderr, "[DEBUG]   ");
                break;
            case LOG_INFO:
                fprintf(stderr, "[INFO]    ");
                break;
            case LOG_WARNING:
                fprintf(stderr, "[WARNING] ");
                break;
            case LOG_ERROR:
                fprintf(stderr, "[ERROR]   ");
                break;
        }
        va_list arguments;
        va_start(arguments, fmt);
        vfprintf(stderr, fmt, arguments);
        va_end(arguments);
        fprintf(stderr,"\n");
    }
}
