#pragma once

#include <arch/i686/ide.h>
#include <stdint.h>
#include <util/list.h>

typedef struct {
    uint64_t start;
    uint64_t size;
    uint64_t attributes;
    uint32_t name_length;
    char *name;
    ATA_Drive *drive;
} Partition;

int disk_read_sectors(
    Partition *partition, uint64_t sectors, uint64_t lba, void *buffer
);
ListNode *disk_get_partitions();
void disk_initialize();