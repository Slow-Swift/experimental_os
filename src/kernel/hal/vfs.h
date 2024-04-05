#pragma once

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

enum StandardStreams {
    STREAM_STDIN = -1,
    STREAM_STDOUT = -2,
    STREAM_STDERR = -3,
    STREAM_STDDBG = -4
};

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
int vfs_write(FILE *file, const uint8_t *data, size_t size);

int vfs_read(FILE *file, char *buff, size_t size);