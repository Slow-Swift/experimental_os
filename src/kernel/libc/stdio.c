#include "stdio.h"

#include "format_print.h"
#include <stdint.h>
#include <stdbool.h>
#include <hal/vfs.h>

static size_t array_max;
static size_t array_index;
static bool limit_array_size;
static char * restrict array_out;

/**
 * Output a char to an array. Only output the char if limit_array_size is
 * false or there is space in the array.
 * 
 * Parameters:
 *   c: The character to print
 *   stream: A stream parameter so this function matches the printchar
 *           definition
*/
static int print_char_to_array(char c, FILE * restrict stream) {
    if (!limit_array_size || (array_index < array_max - 1)) {
        array_out[array_index] = c;
        array_index++;
        return 1;
    }

    return 0;
}

size_t fwrite(
    const void * restrict ptr, size_t size, size_t nmemb, 
    FILE * restrict stream
) {
    const uint8_t *uint8_ptr = (const uint8_t *)(ptr);

    for (size_t i=0; i<nmemb; i++) {
        if(vfs_write(stream, uint8_ptr + size * i, size) != size)
            return i;
    }
    return nmemb;
}

int fputc(int c, FILE *stream) 
{
    unsigned char to_put = c;
    vfs_write(stream, &to_put, sizeof(to_put));
    return to_put;
}

int putc(char c, FILE *stream) 
{
    return fputc(c, stream);
}

int fputs(const char * restrict s, FILE * restrict stream) 
{
    int count = 0;
    while (*s) {
        fputc(*s, stream);
        s++;
        count++;
    }
    return count;
}

int putchar(int c) 
{
    return fputc(c, stdout);
}

int puts(const char *s) 
{
    int count = fputs(s, stdout);
    fputc('\n', stdout);
    return count + 1;
}

int fprintf(FILE * restrict stream, const char * restrict format, ...) 
{
    int result;
    va_list args;
    va_start(args, format);

    result = vfprintf(stream, format, args);

    va_end(args);
    return result;
}

int printf(const char * restrict format, ...) 
{
    int result;

    va_list args;
    va_start(args, format);
    result = vfprintf(stdout, format, args);
    va_end(args);

    return result;
}

int snprintf(char * restrict s, size_t n, const char * restrict format, ...) {
    int result;

    va_list args;
    va_start(args, format);
    result = vsnprintf(s, n, format, args);
    va_end(args);

    return result;
}

int sprintf(char * restrict s, const char * restrict format, ...) {
    int result;

    va_list args;
    va_start(args, format);
    result = vsprintf(s, format, args);
    va_end(args);

    return result;
}

int vfprintf(
    FILE * restrict stream, const char * restrict format, va_list arg
) {
    format_print(format, stream, fputc, arg);
}

int vprintf(const char * restrict format, va_list arg) {
    format_print(format, stdout, fputc, arg);
}

int vsnprintf(
    char * restrict s, size_t n, const char * restrict format, va_list arg
) {
    array_max = n;
    array_index = 0;
    array_out = s;
    limit_array_size = true;
    int result = format_print(format, NULL, print_char_to_array, arg);
    array_out[array_index] = '\0';
    array_out = NULL;
    return result;
}

int vsprintf(
    char * restrict s, const char * restrict format, va_list arg
) {
    array_index = 0;
    array_out = s;
    limit_array_size = false;
    int result = format_print(format, NULL, print_char_to_array, arg);
    array_out[array_index] = '\0';
    array_out = NULL;
    return result;
}

int fprintbuf(FILE * restrict stream, const void *buffer, size_t count) {
    const uint8_t* u8Buffer = (const uint8_t*)buffer;

    for(uint16_t i=0; i<count; i++) {
        fprintf(stream, "%hhx", u8Buffer[i]);
    }
    return count;
}
