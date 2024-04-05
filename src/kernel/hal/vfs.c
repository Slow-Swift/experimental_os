#include "vfs.h"

#include "file.h"
#include <arch/i686/e9.h>
#include <arch/i686/vga_text.h>

#define DEBUG_MODE 1

int vfs_write(FILE *file, const uint8_t *data, size_t size) {
    switch (file->handle) {
    case STREAM_STDIN:
        return 0;
    case STREAM_STDOUT:
    case STREAM_STDERR:
        for (size_t i=0; i<size; i++) {
            vga_putc(data[i]);
#ifdef DEBUG_MODE
            e9_putc(data[i]);
#endif
        }
        return size;
    case STREAM_STDDBG:
        for (size_t i=0; i<size; i++) {
            e9_putc(data[i]);
        }
        return size;
    default:
        if (file->accessors == NULL) return 0;
        if (file->accessors->write == NULL) return 0;
        return file->accessors->write(file, data, size);
    }
}

int vfs_read(FILE *file, char *buff, size_t size) {
    switch (file->handle) {
    case STREAM_STDIN:
        return 0;       // TODO
    case STREAM_STDOUT:
    case STREAM_STDERR:
    case STREAM_STDDBG:
        return 0;
    default:
        if (file->accessors == NULL) return 0;
        if (file->accessors->write == NULL) return 0; 
        return file->accessors->read(file, buff, size);
    }
}