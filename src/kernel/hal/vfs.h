#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
    int file_descriptor;
} FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;
extern FILE* stddbg;

/**
 * Write [size] bytes of data from [data] to [file]
 * 
 * Parameters:
 *   file: The file to write to
 *   data: The start of the data
 *   size: The number of bytes to write
 * 
 * Returns:
 *   int: The number of bytes actually written
*/
int vfs_write(FILE* file, uint8_t* data, size_t size);