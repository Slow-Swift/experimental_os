#include "vfs.h"

#include <arch/i686/e9.h>
#include <arch/i686/vga_text.h>

#define DEBUG_MODE 1

enum StandardStreams {
    STDIN = 0,
    STDOUT = 1,
    STDERR = 2,
    STDDBG = 3
};

FILE stdin_file = { .file_descriptor = STDIN };
FILE stdout_file = { .file_descriptor = STDOUT };
FILE stderr_file = { .file_descriptor = STDERR };
FILE stddbg_file = { .file_descriptor = STDDBG };

FILE *stdin = &stdin_file;
FILE *stdout = &stdout_file;
FILE *stderr = &stderr_file;
FILE *stddbg = &stddbg_file;

int vfs_write(FILE *file, const uint8_t *data, size_t size) {
    switch (file->file_descriptor) {
    case STDIN:
        return 0;
    case STDOUT:
    case STDERR:
        for (size_t i=0; i<size; i++) {
            vga_putc(data[i]);
#ifdef DEBUG_MODE
            e9_putc(data[i]);
#endif
        }
        return size;
    case STDDBG:
        for (size_t i=0; i<size; i++) {
            e9_putc(data[i]);
        }
        return size;
    default:
        return 0;
    }
}