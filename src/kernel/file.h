#pragma once

#include <stddef.h>
#include <stdbool.h>

struct FileData;

typedef struct {
    size_t (*write)(struct FileData *fd, const char *data, size_t size);
    size_t (*read)(struct FileData *fd, char *buff, size_t size);
    int (*flush)(struct FileData *fd);
    int (*close)(struct FileData *fd);
} StreamAccessors;

typedef struct FileData {
    int handle;
    char * buffer;
    int buffer_mode;
    int buffer_size;
    bool buffer_auto_allocated;
    bool opened;
    bool error;
    bool eof;
    StreamAccessors *accessors;
    void *extra_data;
} FileData;