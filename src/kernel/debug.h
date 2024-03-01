#pragma once

#include <stdarg.h>
#include <stddef.h>

#define MIN_LOG_LEVEL 0

typedef enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 3,
    LVL_ERROR = 4,
    LVL_CRITICAL = 5
} DebugLevel;

/**
 * Print a message and then hexdump of [count] bytes of the buffer pointed to 
 * by [buffer].
 * 
 * Parameters:
 *   msg: The message to print
 *   buffer: The buffer to hexdump from
 *   count: The number of bytes to print
*/
void debug_buffer(const char *msg, const void *buffer, size_t count);

/**
 * Log a message with the prefix [module] at debug level [level] and controlled
 * by the format string [format].
 * 
 * Parameters:
 *   module: The module previx to use
 *   level: The debug level to print at
 *   format: The format control string
 *   ...: Arguments to be used by the format string
*/
void logf(const char *module, DebugLevel level, const char *format, ...);

void panic(const char *module, char *format, ...);

/**
 * MACRO. Equivalent to logf with LVL_DEBUG as the level.
*/
#define log_debug(module, ...) logf(module, LVL_DEBUG, __VA_ARGS__)

/**
 * MACRO. Equivalent to logf with LVL_INFO as the level.
*/
#define log_info(module, ...) logf(module, LVL_INFO, __VA_ARGS__)

/**
 * MACRO. Equivalent to logf with LVL_WARN as the level.
*/
#define log_warn(module, ...) logf(module, LVL_WARN, __VA_ARGS__)

/**
 * MACRO. Equivalent to logf with LVL_ERROR as the level.
*/
#define log_error(module, ...) logf(module, LVL_ERROR, __VA_ARGS__)

/**
 * MACRO. Equivalent to logf with LVL_CRITICAL as the level.
*/
#define log_critical(module, ...) logf(module, LVL_CRITICAL, __VA_ARGS__)

