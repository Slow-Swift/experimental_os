#include "stdio.h"

#include "format_print.h"
#include <file.h>
#include <stdint.h>
#include <stdbool.h>
#include <hal/vfs.h>
#include <stdlib.h>
#include <fat.h>

static size_t array_max;
static size_t array_index;
static bool limit_array_size;
static char * restrict array_out;


static FILE stdin_file = { .handle = STREAM_STDIN };
static FILE stdout_file = { .handle = STREAM_STDOUT };
static FILE stderr_file = { .handle = STREAM_STDERR };
static FILE stddbg_file = { .handle = STREAM_STDDBG };

FILE *stdin = &stdin_file;
FILE *stdout = &stdout_file;
FILE *stderr = &stderr_file;
FILE *stddbg = &stddbg_file;

FileData files[FOPEN_MAX];

int feof(FILE *stream) {
    return stream->eof;
}

int ferror(FILE *stream) {
    return stream->error;
}

int fclose(FILE *stream) {
    int ret = fflush(stream);
    if (stream->handle < 0 || stream->handle >= FOPEN_MAX) return EOF;

    if (stream->accessors->close != NULL) 
        stream->accessors->close(stream);
    
    if (stream->buffer != NULL && stream->buffer_auto_allocated) {
        free(stream->buffer);
    }

    stream->buffer = NULL;
    stream->accessors = NULL;
    stream->buffer_auto_allocated = false;
    stream->buffer_mode = _IONBF;
    stream->buffer_size = false;
    stream->opened = false;
    stream->eof = false;
    stream->error = false;
    return ret;
}

int fflush(FILE *stream) {
    if (stream == NULL) {
        for (int i=0; i<FOPEN_MAX; i++) {
            if (stream->opened) {
                fflush(stream);
            }
        }
        return 0;
    }

    if (stream->accessors != NULL && stream->accessors->flush != NULL)
        stream->accessors->flush(stream);
    return 0;
}

FILE *fopen(const char * restrict filename, const char * restrict mode) {
    FileData *stream = NULL;
    for (int i=0; i<FOPEN_MAX; i++) {
        if (!files[i].opened) {
            stream = files + i;
            stream->handle = i;
            stream->eof = false;
            stream->error = false;
            break;
        }
    }
    if (stream == NULL) return NULL;
    if (fat_open_file(stream, filename, mode) == 0) {
        stream->opened = true;
        return stream;
    }
    return NULL;
}

void setbuf(FILE * restrict stream, char * restrict buf) {
    if (stream->buffer != NULL) return;

    if (buf == NULL) {
        stream->buffer_mode = _IONBF;
        stream->buffer_size = 0;
    }
    else {
        stream->buffer = buf;
        stream->buffer_mode = _IOFBF;
        stream->buffer_size = BUFSIZ;
    }
    stream->buffer_auto_allocated = false;
}

int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size) 
{
    if (stream->buffer != NULL) return -1; // Buffer already set
    if (mode <= 0 || mode > 3) return -2; // Invalid mode

    if (buf == NULL) {
        stream->buffer_auto_allocated = true;
        stream->buffer = malloc(size);
        if (stream->buffer == NULL) return -3;
    } else {
        stream->buffer_auto_allocated = false;
        stream->buffer = buf;
    }

    stream->buffer_mode = mode;
}

/**
 * Output a char to an array. Only output the char if limit_array_size is
 * false or there is space in the array.
 * 
 * Parameters:
 *   c: The character to print
 *   stream: A stream parameter so this function matches the printchar
 *           definition
*/
static int print_char_to_array(int c, FILE * restrict stream) {
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

size_t fread(
    void * restrict ptr, size_t size, size_t nmemb, 
    FILE * restrict stream
) {
    uint8_t *uint8_ptr = (uint8_t *)(ptr);

    for (size_t i=0; i<nmemb; i++) {
        if(vfs_read(stream, uint8_ptr + size * i, size) != size)
            return i;
    }
    return nmemb;
}

int fgetc(FILE * stream) 
{
    unsigned char read;
    if(vfs_read(stream, &read, sizeof(read)) != sizeof(read))
        return EOF;
    return read;
}

char *fgets(char * restrict s, int n, FILE * stream) 
{
    int i=0;
    for (; i<n-1; i++) {
        char c = fgetc(stream);
        if (c == EOF) break;;
        s[i] = c;
        if (c == '\n') {
            i++;
            break;
        }
    }
    if (i==0) return NULL;
    s[i] = '\0';
    return s;
}

int fputc(int c, FILE * restrict stream) 
{
    unsigned char to_put = c;
    if(vfs_write(stream, &to_put, sizeof(to_put)) != sizeof(to_put))
        return EOF;
    return to_put;
}

int putc(char c, FILE * restrict stream) 
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

int hexdump(FILE * restrict stream, const void *buffer, size_t count) {
    int offset = 0;
    for (int offset = 0; offset < count; offset += 8) {
        fprintf(stream, "%#x: \t", offset);
        for (int col=0; (col < 8) && (col < count); col++)
            printf("%02x ", ((uint8_t *)buffer)[offset + col]);
        printf("| ");
        for (int col=0; (col < 8) && (col < count); col++) {
            uint8_t c = ((uint8_t *)buffer)[offset + col];
            char to_print = ' ';
            if (0x30 <= c && c <= 0x7D) to_print = c;
            fputc(to_print, stream);
        }
        printf("\n");
    }

    return count;
}