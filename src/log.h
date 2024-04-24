#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

/** @defgroup log log
 *
 *   This module contains some formatted logging functionality
 *
 *  @{
 */

typedef enum {
    LOG_DEBUG = 10,
    LOG_INFO = 20,
    LOG_WARNING = 30,
    LOG_ERROR = 40
} log_level_t;

/** @brief global log_level */
extern log_level_t log_level;

/**
 * @brief print a formatted log message
 *
 * Prints a the log message defined by the format string fmt and the variable
 * arguments if the log level is higher than the global log level log_level
 *
 * @param level log level for the message
 * @param fmt a format string in the printf style
 * @param ... format string parameters
 */
void print_log(log_level_t level, char const* fmt, ...);

/**@}*/

#endif
