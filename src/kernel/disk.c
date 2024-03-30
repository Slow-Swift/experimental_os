#include "disk.h"

#include <arch/i686/ide.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util/list.h>

static const uint64_t ELF_MAGIC = 0x5452415020494645; // "EFI PART"

typedef struct {
    char signature[8];
    uint32_t revision;
    uint32_t size;
	uint32_t checksum;
	uint32_t reserved;
	uint64_t header_lba;
	uint64_t alt_header_lba;
	uint64_t usable_start;
	uint64_t usable_end;
	char guid[16];
	uint64_t entry_start_lba;
	uint32_t partition_entry_count;
    uint32_t entry_size;
    uint32_t entry_array_cr2;
} __attribute__((packed)) GPT_PartitionTableHeader;

typedef struct {
    char type_guid[16];
    char guid[16];
    uint64_t lba_start;
    uint64_t lba_end;
    uint64_t attributes;
    char name[72];
} GPT_Partition;

typedef struct {
    uint64_t start;
    uint64_t size;
    uint64_t attributes;
    uint32_t name_length;
    char *name;
    ATA_Drive *drive;
} Partition;

static ListNode *partitions;
static int partition_count;

static void read_gpt(ATA_Drive *drive) {
    uint8_t *buffer = malloc(512);
    if (buffer == NULL) {
        printf("  ERROR: Could not allocate memory for disk buffer\n");
        return;
    }

    GPT_PartitionTableHeader *header = (GPT_PartitionTableHeader *)buffer;
    partition_count = 0;

    // Read in the header
    if (!drive->read_sectors(1, 1, buffer)) {
        printf("  ERROR: Disk read failed");
        free(buffer);
        return;
    }


    if (*((uint64_t*)(header->signature)) != ELF_MAGIC) {
        printf("  DISK ERROR: GPT Header Not Valid");
        free(buffer);
        return;
    }

    uint64_t lba_start = header->entry_start_lba;
    uint64_t lba = 0;
    uint32_t entry_count = header->partition_entry_count;
    uint32_t entry_size = header->entry_size;
    uint32_t name_size = entry_size - 0x38;
    GPT_Partition *gpt_partition;
    Partition *partition;

    for (uint64_t i = 0; i < entry_count; i++) {
        uint64_t target_lba = (i * entry_size) / 512 + lba_start;
        uint64_t offset = (i * entry_size) % 512;

        // Ensure sector is loaded
        if (lba != target_lba) {
            lba = target_lba;
            if (!drive->read_sectors(1, lba, buffer)) {
                printf("  ERROR: Disk read failed");
                free(buffer);
                return;
            }
        }

        gpt_partition = (GPT_Partition *)(buffer + offset);

        // Check that entry is used
        char tguid_present = 0;
        for (int j=0; j<16; j++) tguid_present |= gpt_partition->type_guid[j];
        if (!tguid_present) continue;

        // Copy info to the partition object
        partition = malloc(sizeof(*partition));
        char *name = malloc(name_size);
        if (partition == NULL || name == NULL) {
            printf("  ERROR: Could not allocate memory for partition");
            free(buffer);
            return;
        }

        partition->start = gpt_partition->lba_start;
        partition->size = gpt_partition->lba_end - gpt_partition->lba_start;
        partition->attributes = gpt_partition->attributes;
        partition->name_length = name_size;
        partition->name = name;
        partition->drive = drive;
        memcpy(name, gpt_partition->name, name_size);
        list_add_tail(&partitions, partition);
        partition_count++;
    }

    free(buffer);
}

int disk_read_sectors(Partition *partition, int sectors, int lba, void *buffer) {
    if (lba >= partition->size) return 0;

    return partition->drive->read_sectors(
            sectors, lba + partition->start, buffer);
}

void disk_initialize() {
    printf("\n");
    printf("Initializing Disk\n");

    ATA_Drive *drive;
    for (int i=0; i<4; i++) {
        drive = ide_get_drive(i);
        if (drive != NULL && drive->type == ATA_TYPE_PATA) break;
    }

    if (drive == NULL) {
        printf("  ERROR: No Disk Found");
        return;
    }

    read_gpt(drive);
    printf("  Found %d partition(s)\n", partition_count);
    ListNode *current = partitions;
    int i=0;
    while (current) {
        Partition *part = current->value;
        printf("  Partition %d:\n", i);
        printf("  | Name: ");
        for (int j=0; j<part->name_length; j++)
            if (part->name[j] != '\0') putc(part->name[j], stdout);
        printf("\n");
        printf("  | LBA Start: %#llx\n", part->start);
        printf("  | Size Sectors: %#llx\n", part->size);
        printf("  | Size: %lld MB\n", part->size / 2 / 1024);
        current = current->next;
        i++;
    }

    printf("Initialized Disk\n");
}