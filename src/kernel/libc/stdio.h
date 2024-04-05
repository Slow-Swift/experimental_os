#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint64_t fpos_t;

// FILE defined in <hal/vfs.h>
// stdin, stdout, stderr, stddbg defined in <hal/vfs.h>

#define EOF -1  
#define _IOFBF 1
#define _IOLBF 2
#define _IONBF 3

#define BUFSIZ 512
#define FOPEN_MAX 16
#define FILENAME_MAX 256

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 3

typedef struct FileData FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
extern FILE* stddbg;

// TODO STD: define L_tmpnam
// TODO STD: define TMP_MAX

// TODO STD: define remove
// TODO STD: define rename
// TODO STD: define tmpfile
// TODO STD: define tmpnam

/**
 * Close a file, flushing all buffers
 * 
 * Parameters:
 *   stream: The filestream to close
 * 
 * Returns:
 *  0 on success, EOF on error
*/
int fclose(FILE *stream);

/**
 * Flush a filestream, writing all buffered data
 * 
 * Parameters:
 *   The stream to flush
 * 
 * Returns:
 *   0 on success, EOF on error
*/
int fflush(FILE *stream);


/**
 * Open a file with the given modes
 * 
 * Parameter:
 *   filename: The name of the file to open
 *   mode: The mode to open the file in
 * 
 * Returns:
 *   The opened filestream on success or NULL on error
*/
FILE *fopen(const char * restrict filename, const char * restrict mode);

// TODO STD: define freopen

/**
 * Set the buffer and buffer of a filestream. If the buffer is not null then
 * _IOFBF is used as the mode otherwise _IONBF is used. This is only valid as
 * the first operation a file stream. Does not return an error on failure.
 * 
 * Parameters:
 *   stream: The stream to set the buffer for
 *   buf: The buffer to set
*/
void setbuf(FILE * restrict stream, char * restrict buf);

/**
 * Set the buffer and buffer mode of a file stream. May only be called as the 
 * first operation on a filestream
 * 
 * Parameters:
 *   stream: The stream to set the buffer for
 *   buf: The buffer to set
 *   mode: The buffer mode to use
 *   size: The size of the buffer
 * 
 * Returns:
 *   0 on success
 *   -1 if invalid call
 *   -2 for invalid mode
 *   -3 on memory allocation fail
*/
int setvbuf(FILE * restrict stream, char * restrict buf, int mode, size_t size);

// TODO STD: define fscanf
// TODO STD: define scanf
// TODO STD: define sscanf
// TODO STD: define vfscanf
// TODO STD: define vscanf
// TODO STD: define vsscanf

/**
 * Read in a character from a file stream
 * 
 * Parameters:
 *   stream: The stream to read from
 * 
 * Returns:
 *   the read character, or EOF on error
*/
int fgetc(FILE * stream);

/**
 * Read up to n-1 characters from a filestream into the array pointed to by s
 * 
 * Parameters:
 *   s: The array to read into
 *   n: The maximum number of characters to read + 1 for the null byte
 *   stream: The stream to read from
 * 
 * Returns:
 *   s on success, NULL on error
*/
char *fgets(char * restrict s, int n, FILE * stream);

// TODO STD: define getc
// TODO STD: define getchar
// TODO STD: define ungetc


/**
 * Read to the array pointed to by [ptr] up to [nmemb] elements whose
 * size is specified by [size] from the stream pointed to by [stream].
 * 
 * Parameters:
 *   ptr: base to the start of the array
 *   size: The size of a member of the array
 *   nmemb: The number of memebers to copy
 *   stream: The file stream to write to
 * 
 * Returns (size_t):
 *   The number of members actually read. Only less than nmemb if an
 *   error occurred.
*/
size_t fread(
    void * restrict ptr, size_t size, size_t nmemb, 
    FILE * restrict stream
);

// TODO STD: define fgetpos
// TODO STD: define fseek
// TODO STD: define fsetpos
// TODO STD: define ftell
// TODO STD: define frewind

// TODO STD: define clearerr
// TODO STD: define feof
// TODO STD: define ferror
// TODO STD: define perror

/**
 * Writes from the array pointed to by [ptr] up to [nmemb] elements whose
 * size is specified by [size] to the stream pointed to by [stream].
 * 
 * Parameters:
 *   ptr: base to the start of the array
 *   size: The size of a member of the array
 *   nmemb: The number of memebers to copy
 *   stream: The file stream to write to
 * 
 * Returns (size_t):
 *   The number of members actually written. Only less than nmemb if an
 *   error occurred.
*/
size_t fwrite(
    const void * restrict ptr, size_t size, size_t nmemb, 
    FILE * restrict stream
);

/**
 * Writes the character specified by [c] (converted to an unsigned char)
 * to the output stream specified by
 * [stream], at the position indicated by the associated file position
 * indicator for the stream (if defined), and advances the indicator
 * appropriately. If the file cannot support positioning requests, or if the 
 * stream was opened with append mode, the character is appended to the output 
 * stream.
 * 
 * Parameters:
 *   c: The character to write
 *   stream: The file stream to write to
 * 
 * Returns (int):
 *   The character written if there was no error. 
 *   EOF if there was an error.
*/
int fputc(int c, FILE *stream);

/**
 * A call to fputc. Always use fputc instead
*/
int putc(char c, FILE *stream);

/**
 * Writes the character specified by [c] (converted to an unsigned char)
 * to the stdout.
 * 
 * Parameters:
 *   c: The character to write
 * 
 * Returns (int):
 *   The character written if there was no error. 
 *   EOF if there was an error.
*/
int putchar(int c);

/**
 * Writes the string pointed to by [s] to the stream pointed to by [stream].
 * The termination character is not written.
 * 
 * Parameters:
 *   s: The string to write
 *   stream: The stream to write to
 * 
 * Returns:
 *   A non-negative value if no error occurs
 *   EOF if an error occurs
*/
int fputs(const char * restrict s, FILE * restrict stream);

/**
 * Write the string pointed to by [s] to stdout and append a newline character
 * to the output. The termination character is not written.
 * 
 * Parameters:
 *   s: The string to print
 * 
 * Returns:
 *   A non-negative value if no error occurs
 *   EOF if an error occurs
*/
int puts(const char *s);

/**
 * Writes output to the stream pointed to by [stream], under control
 * of the string pointed to by [format] that specifies how subsequent arguments 
 * are converted for output. 
 * 
 * Parameters:
 *   stream: The stream to write to
 *   format: The format string
 *   ...: The arguments for the format string
 * 
 * Returns:
 *   The number of characters outputted if no error occurs
 *   A negative value if an error occurs
*/
int fprintf(FILE * restrict stream, const char * restrict format, ...);

/**
 * Equivalent to fprintf with the first argument as stdout
*/
int printf(const char * restrict format, ...);

/**
 * Equivalent to fprintf except the output is written into an array specified
 * by [s]. Any characters beyond [n-1] are discarded and a null character is 
 * written at the end of the characters acrually written into the array.
 * 
 * Parameters:
 *   s: The array to write to
 *   n: The maximum number of characters to write into the array
 *   format: The format string
 *   ...: The arguments for the format string
 * 
 * Returns int:
 *  The number of characters that would have been outputted if n was large
 *  enough if no error occurred
 *  A negative value if an error occurred
*/
int snprintf(char * restrict s, size_t n, const char * restrict format, ...);

/**
 * Equivalent to fprintf except the output is written into an array specified
 * by [s]. A null character is written at the end of the characters written; 
 * it is not counted as part of the returned value.
 * 
 * Parameters:
 *   s: The array to write to
 *   format: The format string
 *   ...: The arguments for the format string
 * 
 * Returns int:
 *  The number of characters written to the array if no error occurred
 *  A negative value if an error occurred
*/
int sprintf(char * restrict s, const char * restrict format, ...);

/**
 * Equivalent to fprintf, with the variable argument list replaced by arg.
 * 
 * Parameters:
 *   stream: The stream to write to
 *   format: The format string
 *   arg: The arguments for the format string
 * 
 * Returns:
 *   The number of characters outputted if no error occurs
 *   A negative value if an error occurs
*/
int vfprintf(FILE * restrict stream, const char * restrict format, va_list arg);

/**
 * Equivalent to printf, with the variable argument list replaced by arg.
 * 
 * Parameters:
 *   format: The format string
 *   arg: The arguments for the format string
 * 
 * Returns:
 *   The number of characters outputted if no error occurs
 *   A negative value if an error occurs
*/
int vprintf(const char * restrict format, va_list arg);

/**
 * Equivalent to snprintf, with the variable argument list replaced by arg.
 * 
 * Parameters:
 *   s: The array to write to
 *   format: The format string
 *   arg: The arguments for the format string
 * 
 * Returns:
 *  The number of characters written to the array if no error occurred
 *  A negative value if an error occurred
*/
int vsnprintf(
    char * restrict s, size_t n, const char * restrict format, va_list arg
);

/**
 * Equivalent to sprintf, with the variable argument list replaced by arg.
 * 
 * Parameters:
 *   s: The array to write to
 *   n: The maximum number of characters to write to the array
 *   format: The format string
 *   arg: The arguments for the format string
 * 
 * Returns:
 *  The number of characters written to the array if no error occurred
 *  A negative value if an error occurred
*/
int vsprintf(
    char * restrict s, const char * restrict format, va_list arg
);

/**
 * Output [count] bytes of the buffer pointed to by [buffer] in hexadecimal
 * form to the stream pointed to by [stream]
 * 
 * Parameters:
 *   stream: The stream to write to
 *   buffer: The buffer to write
 *   count: The number of bytes to run
 * 
 * Return int:
 *   The number of bytes of the buffer actually written. Only less than count
 *   if there was an error.
*/
int fprintbuf(FILE * restrict stream, const void *buffer, size_t count);

/**
 * Output [count] bytes of the buffer pointed to by [buffer] formatted nicely
 * addr: xx xx xx xx   xx xx xx xx abcd efgh
 * 
 * Parameters:
 *   stream: The stream to write to
 *   buffer: The buffer to write
 *   count: The number of bytes to run
 * 
 * Return int:
 *   The number of bytes of the buffer actually written. Only less than count
 *   if there was an error.
*/
int hexdump(FILE * restrict stream, const void *buffer, size_t count);