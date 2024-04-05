#pragma once

#include "file.h"

void fat_initialize();
int fat_open_file(
    FileData *fd, const char * restrict filename, const char * restrict mode);