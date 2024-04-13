#include "fat.h"

#include "disk.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint32_t sectors_per_fat_32;
    uint16_t flags;
    uint16_t fat_version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector;
    char reserved[12];
    uint8_t drive_number;
    uint8_t flags_nt;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char system_identifier[8];
} __attribute__((packed)) EBR_32;

typedef struct {
    uint8_t drive_number;
    uint8_t flags_nt;
    uint8_t signature;
    uint32_t volume_id;
    char volume_label[11];
    char system_identifier[8];
} __attribute__((packed)) EBR_16;

typedef struct {
    char short_jump[3];
    char oem_identifier[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_count;
    uint16_t root_entry_count;
    uint16_t sector_count_16;
    uint8_t media_type;
    uint16_t sectors_per_fat_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sector_count;
    uint32_t sector_count_32;
    union {
        EBR_16 ebr_16;
        EBR_32 ebr_32;
    };
} __attribute__((packed)) BootRecord;

typedef struct {
    uint32_t signature;
    char reserved[480];
    uint32_t signature_2;
    uint32_t last_free_cluster;
    uint32_t free_cluster;
} __attribute__((packed)) FS_Info;

typedef struct {
    char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t created_time_tenths;
    uint16_t created_time;
    uint16_t created_date;
    uint16_t accessed_date;
    uint16_t first_cluster_high;
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed)) DirEntry;

typedef struct {
    uint8_t order;
    char chars0_4[10];
    uint8_t attribute;
    uint8_t checksum;
    char chars5_10[12];
    uint16_t reserved;
    char chars11_12[4];
} __attribute__((packed)) LongFilename;

typedef enum {
    FAT_12,
    FAT_16,
    FAT_32,
    EXFAT,
} FatFormat;

typedef enum {
    ATTRIB_READ_ONLY    = 0x01,
    ATTRIB_HIDDEN       = 0x02,
    ATTRIB_SYSTEM       = 0x04,
    ATTRIB_VOLUME_ID    = 0x08,
    ATTRIB_DIRECTORY    = 0x10,
    ATTRIB_ARCHIVE      = 0x20,
} DirEntryAttribs;

typedef struct {
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t cluster_index;
    uint32_t current_sector_in_cluster;
    uint32_t position;
    uint32_t size;
    bool directory;
} FatFileData;

#define SECTOR_SIZE 512

Partition *partition;

BootRecord boot_record;
uint8_t fat_buffer[512];
uint32_t fat_sector;

FatFormat format;
uint32_t total_sectors;
uint32_t total_clusters;
uint32_t first_fat_sector;
uint32_t first_data_sector;

static size_t fat_write(FileData *fd, const char *data, size_t size);
static size_t fat_read(FileData *fd, char *buff, size_t size);
static int fat_flush(FileData *fd);
static int fat_close(FileData *fd);

StreamAccessors stream_accessors = {
    .write = fat_write,
    .read = fat_read,
    .flush = fat_flush,
    .close = fat_close
};

static uint32_t cluster_to_lba(int cluster) {
    return first_data_sector + (cluster - 2) * boot_record.sectors_per_cluster;
}

static int read_cluster(uint32_t cluster, uint8_t sector, void *buffer) {
    return disk_read_sectors(
        partition, 1, cluster_to_lba(cluster) + sector, buffer);
}

static uint32_t next_cluster(uint32_t current_cluster) {
    uint32_t fat_offset = current_cluster << 2;
    uint32_t target_fat_sector = first_fat_sector + (fat_offset / SECTOR_SIZE);
    uint32_t entry_offset = fat_offset % SECTOR_SIZE;

    if (fat_sector != target_fat_sector) {
        if (!disk_read_sectors(partition, 1, target_fat_sector, fat_buffer)) {
            printf("DISK READ ERROR: Could not read fat sector");
            fat_sector = 0; // Invalid sector, will be reloaded on next read
            return 0x0FFFFFF7;
        }
        fat_sector = target_fat_sector;
    }

    uint32_t table_value = *(uint32_t *)(fat_buffer + entry_offset);
    return table_value & 0x0FFFFFFF;
}

/**
 * Fill the file buffer.
 * 
 * Parameters:
 *   fd: The file stream to advance to the next cluster
 * 
 * Returns:
 *   0 on success, EOF on error
*/
static int load_file_buffer(FileData *fd) {
    FatFileData *ffd = (FatFileData *)fd->extra_data;
    uint32_t target_sector = ffd->position / SECTOR_SIZE;
    uint32_t target_cluster_index = 
        target_sector / boot_record.sectors_per_cluster;
    target_sector = target_sector % boot_record.sectors_per_cluster;
    
    // If position is further back in the file then seek from the start
    if (target_cluster_index < ffd->cluster_index) {
        ffd->cluster_index = 0;
        ffd->current_cluster = ffd->first_cluster;
    }

    // Find the correct cluster
    while(ffd->cluster_index < target_cluster_index) {
        ffd->cluster_index++;
        ffd->current_cluster = next_cluster(ffd->current_cluster);
        if (ffd->current_cluster >= 0x0FFFFFF7) return EOF;
    }
    ffd->current_sector_in_cluster = target_sector;

    // Read into the file buffer
    if (read_cluster(
        ffd->current_cluster, ffd->current_sector_in_cluster, fd->buffer
    ) != 1) return EOF;
    return 0;
}

static void fat_init_partition(Partition *part) {
    // Read the boot record
    if (!disk_read_sectors(part, 1, 0, fat_buffer)) {
        printf("  ERROR: Disk read failed.");
        return;
    }

    memcpy(&boot_record, fat_buffer, sizeof(boot_record));

    // Parse teh exfat later
    if (boot_record.bytes_per_sector == 0) {
        printf("  EXFAT Detected. Not supported yet.\n");
        return;
    }

    total_sectors = (boot_record.sector_count_16 == 0) 
        ? boot_record.sector_count_32 
        : boot_record.sector_count_16;

    uint32_t fat_size = (boot_record.sectors_per_fat_16 == 0)
        ? boot_record.ebr_32.sectors_per_fat_32
        : boot_record.sectors_per_fat_16;

    // On FAT 32 this will be 0
    uint32_t root_dir_sectors = 
        (boot_record.root_entry_count * 32 + boot_record.bytes_per_sector - 1) /
        boot_record.bytes_per_sector;

    first_fat_sector = boot_record.reserved_sector_count;

    first_data_sector = first_fat_sector + 
        boot_record.fat_count * fat_size + root_dir_sectors;

    uint32_t data_sectors = total_sectors - first_data_sector;
    total_clusters = data_sectors / boot_record.sectors_per_cluster;

    // TODO: Verify boot record including that drive number matches bios
    bool br_ok = true;
    br_ok &= total_sectors > 0;
    br_ok &= fat_size > 0;
    br_ok &= first_fat_sector > 0;
    br_ok &= total_clusters > 0;
    br_ok &= boot_record.fat_count > 0;
    br_ok &= boot_record.bytes_per_sector == 512;
    br_ok = 
        boot_record.ebr_16.signature == 0x28 || 
        boot_record.ebr_16.signature == 0x29 ||
        boot_record.ebr_32.signature == 0x28 ||
        boot_record.ebr_32.signature == 0x29;
    
    if (!br_ok) {
        printf("  Boot Record Invalid\n");
        return;
    }

    format = FAT_32;
    if (total_sectors < (0xFFFF-10) && boot_record.ebr_16.signature == 0x28 || 
        boot_record.ebr_16.signature == 0x29
    ) {
        if (total_sectors < (0x3FFF-10)) format == FAT_12;
        else format = FAT_16;
    }

    printf("  Format: %s\n", 
        (const char *[]){"FAT 12", "FAT 16", "FAT 32"}[format]);
    printf("  Total Sectors: %#x\n", total_sectors);
    printf("  Total Clusters: %#x\n", total_clusters);
    printf("  Fat Size: %#x\n", fat_size);

    partition = part;
}

static size_t fat_write(FileData *fd, const char *data, size_t size) {
    return 0;
}

static size_t fat_read(FileData *fd, char *buff, size_t size) {
    FatFileData *ffd = (FatFileData *)fd->extra_data;
    size_t remaining = size;
    size_t remaining_in_file = ffd->size - ffd->position;

    if (remaining_in_file == 0) {
        fd->eof = true;
        return 0;
    }

    if (fd->error) return 0;

    while (remaining > 0) {
        size_t remaining_in_sector = SECTOR_SIZE - ffd->position % SECTOR_SIZE;
        size_t to_read = remaining > remaining_in_sector ? 
            remaining_in_sector : remaining;
        to_read = to_read > remaining_in_file ? remaining_in_file : to_read;
        memcpy(buff, fd->buffer + ffd->position % SECTOR_SIZE, to_read);
        remaining -= to_read;
        remaining_in_file -= to_read;
        buff += to_read;
        ffd->position += to_read;
        if (remaining_in_file == 0) {
            if (remaining > 0) fd->eof = true;
            break;
        }

        if (to_read >= remaining_in_sector) {
            if (load_file_buffer(fd) != 0) {
                fd->error = true;
                return size - remaining;
            }
        }
    }
    return size - remaining;
}

static int fat_flush(FileData *fd) {
    return EOF;
}

static int fat_close(FileData *fd) {
    if (fd->extra_data != NULL) free(fd->extra_data);
}

/**
 * Open the root directory
 * 
 * Parameters:
 *   The filestream to associate with the root directory
 * 
 * Returns:
 *   0 on success, EOF on failure
*/
static int open_root_directory(FileData *fd) {
    FatFileData *ffd = (FatFileData *)fd->extra_data;
    ffd->directory = true;
    ffd->first_cluster = boot_record.ebr_32.root_cluster;
    ffd->current_cluster = ffd->first_cluster;
    ffd->cluster_index = 0;
    ffd->position = 0;
    ffd->size = 0;
    return load_file_buffer(fd);
}


//* Not Yet Complete
static void convert_lfn(LongFilename *lfn) {
    uint16_t str[] = {
        lfn->chars0_4[0],
        lfn->chars0_4[1],
        lfn->chars0_4[2],
        lfn->chars0_4[3],
        lfn->chars0_4[4],
        lfn->chars5_10[5],
        lfn->chars5_10[6],
        lfn->chars5_10[7],
        lfn->chars5_10[8],
        lfn->chars5_10[9],
        lfn->chars5_10[10],
        lfn->chars11_12[11],
        lfn->chars11_12[12],
    };

    int char_index = 0;
    while (char_index < 13) {

    }
}

/**
 * Search for the file called by filename in the top-level of the directory
 * opened in dir.
 * 
 * Parameters:
 *   dir: The stream with the directory, and to open the file in
 *   filename: The name of the file to open
 *   name_length: The length of the filename
 * 
 * Returns:
 *   0 on success, EOF on error
*/
static DirEntry *find_file_in_dir(
    FileData *dir, const char *filename, size_t name_length
) {
    FatFileData *ffd = (FatFileData *)dir->extra_data;
    if (!ffd->directory) return NULL;

    bool more_entries = true;
    int entries_per_sector = SECTOR_SIZE / 32;

    // While there are more entries in the directory...
    while (ffd->current_cluster < 0x0FFFFFF7 && more_entries) {
        DirEntry *entries = (DirEntry *)dir->buffer;

        // Foreach entry in the sector
        for (int i=0; i<entries_per_sector; i++) {
            DirEntry *entry = entries + i;
            // If first byte is 0 then there are no more entries
            if (!entry->name[0]) {
                more_entries = false;
                break;
            }

            if (entry->name[0] == 0xE5) continue;   // Unused entry
            if (entry->attributes == 0x0F) continue;

            // If the name matches then the entry has been found
            if (memcmp(filename, entry->name, 11) == 0) return entry;
        }

        // Read the next sector
        if (!more_entries) {
            ffd->position += SECTOR_SIZE;
            load_file_buffer(dir);
        }
    }
    return NULL;
}

/**
 * Search for the file called by filename in the top-level of the directory
 * opened in dir. Open that file and associate it with the stream str.
 * 
 * Parameters:
 *   dir: The stream with the directory, and to open the file in
 *   filename: The name of the file to open
 *   name_length: The length of the filename
 * 
 * Returns:
 *   0 on success, EOF on error
*/
static int open_file_in_directory(
    FileData *dir, const char *filename, size_t name_length
) {
    FatFileData *ffd = (FatFileData *)dir->extra_data;

    // Find the file entry
    DirEntry *file_entry = find_file_in_dir(dir, filename, name_length);
    if (file_entry == NULL) return EOF;

    uint32_t cluster = (uint32_t)file_entry->first_cluster_high << 16;
    cluster |= file_entry->first_cluster_low;

    // Initialize the fat file data with information about this file
    ffd->first_cluster = cluster;
    ffd->current_cluster = cluster;
    ffd->current_sector_in_cluster = 0;
    ffd->cluster_index = 0;
    ffd->directory = (file_entry->attributes & ATTRIB_DIRECTORY) != 0;
    ffd->position = 0;
    ffd->size = file_entry->size;
    return load_file_buffer(dir);
}

/**
 * Find the file at filename and associate the stream fd with that file.
 * 
 * Parameters:
 *   fd: The filestream to open the file to
 *   filename: The name of the file to open
 *   mode: The mode to open the file with
 * 
 * Returns:
 *   0 on success, EOF on error
*/
int fat_open_file(
    FileData *fd, const char * restrict filename, const char * restrict mode
) {
    // Setup the filestream for a FAT file
    fd->accessors = &stream_accessors;
    fd->extra_data = malloc(sizeof(FatFileData));
    setvbuf(fd, NULL, _IOFBF, SECTOR_SIZE);

    // Start with the root directory
    if(open_root_directory(fd) != 0) return EOF;

    // For each directory in the filename...
    size_t len = strcspn(filename, "/");
    while (len > 0) {
        // ...open the next file/directory
        if (open_file_in_directory(fd, filename, len) != 0) return EOF;
        filename += len;
        if (*filename == '/') filename++;
        len = strcspn(filename, "/");
    }

    return 0;
}

void fat_initialize() {
    printf("\n");
    printf("Initializing FAT\n");

    ListNode *partition_node = disk_get_partitions();
    while (partition_node != NULL && partition == NULL) {
        fat_init_partition(partition_node->value);
        partition_node = partition_node->next;
    }

    if (partition == NULL) {
        printf("  No Valid Partition Found\n");
        return;
    }

    printf("Initialized FAT\n");
}