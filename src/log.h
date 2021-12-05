#ifndef LOG_H
#define LOG_H

typedef enum {
    LOG_DEBUG   = 10,
    LOG_INFO    = 20,
    LOG_WARNING = 30,
    LOG_ERROR   = 40
} log_level_t;

extern log_level_t log_level;

void print_log(log_level_t level, const char * fmt, ...);

#endif
