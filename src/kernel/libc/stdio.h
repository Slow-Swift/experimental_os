#pragma once

#include <hal/vfs.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

// FILE defined in <hal/vfs.h>
// stdin, stdout, stderr, stddbg defined in <hal/vfs.h>

#define EOF -1  

// TODO STD: define fpos_t
// TODO STD: define _IOFBF
// TODO STD: define _IOLBF
// TODO STD: define _IONBF
// TODO STD: define BUFSIZ
// TODO STD: define FOPENMAX
// TODO STD: define FILENAMEMAX
// TODO STD: define L_tmpnam
// TODO STD: define SEEK_CUR
// TODO STD: define SEEK_END
// TODO STD: define SEEK_SET
// TODO STD: define TMP_MAX

// TODO STD: define remove
// TODO STD: define rename
// TODO STD: define tmpfile
// TODO STD: define tmpnam

// TODO STD: define fclose
// TODO STD: define fflush
// TODO STD: define fopen
// TODO STD: define freopen
// TODO STD: define setbuf
// TODO STD: define setvbuf

// TODO STD: define fscanf
// TODO STD: define scanf
// TODO STD: define sscanf
// TODO STD: define vfscanf
// TODO STD: define vscanf
// TODO STD: define vsscanf

// TODO STD: define fgetc
// TODO STD: define fgets
// TODO STD: define getc
// TODO STD: define getchar
// TODO STD: define ungetc

// TODO STD: define fread

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
 *   ptr: Ptr to the start of the array
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