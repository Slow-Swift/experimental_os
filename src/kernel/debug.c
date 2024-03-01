#include "debug.h"

#include <stdio.h>

static const char* const log_severity_colors[] = {
    [LVL_DEBUG]         ="\033[2;37m",
    [LVL_INFO]          ="\033[37m",
    [LVL_WARN]          ="\033[1;33m",
    [LVL_ERROR]         ="\033[1;31m",
    [LVL_CRITICAL]      ="\033[1;37;41m",
};

static const char* const color_reset = "\033[0;0;0m";

void debug_buffer(const char * restrict msg, const void *buffer, size_t count) {
    fputs(msg, stddbg);
    fprintbuf(stddbg, buffer, count);
    fputc('\n', stddbg);
}

void logf(const char *module, DebugLevel level, const char *format, ...) {
    va_list args;
    va_start(args, format);

    if (level < MIN_LOG_LEVEL) return;

    fputs(log_severity_colors[level], stddbg);
    fprintf(stddbg, "[%s] ", module);
    vfprintf(stddbg, format, args);
    fputs(color_reset, stddbg);
    fputc('\n', stddbg);

    va_end(args);
}

void panic(const char *module, char *format, ...) {
    va_list args;
    va_start(args, format);

    fputs(log_severity_colors[LVL_CRITICAL], stddbg);
    fprintf(stddbg, "Panicking:\n  ");
    fprintf(stddbg, "[%s] ", module);
    vfprintf(stddbg, format, args);
    fputc('\n', stddbg);
    fprintf(stderr, "Panic Halt");
    fputs(color_reset, stddbg);
    fputc('\n', stddbg);

    fprintf(stderr, "Panicking:\n  ");
    fprintf(stderr, "[%s] ", module);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    fprintf(stderr, "Panic Halt");

    va_end(args);

    for (;;);
}
