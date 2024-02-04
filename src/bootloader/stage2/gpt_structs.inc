struc partition_table_header
    .magic:             resb 8
    .revision:          resb 4
    .header_size:       resb 4
    .header_checksum:   resb 4
    .reserved:          resb 4
    .header_lba:        resb 8
    .alt_header_lba:    resb 8
    .usable_start:      resb 8
    .usable_end:        resb 8
    .guid:              resb 16
    .part_array_lba:    resb 8
    .partition_count:   resb 4
    .part_entry_size:   resb 4
    .part_array_crc32:  resb 4
endstruc

struc partition_entry
    .type_guid:         resb 16
    .part_guid:         resb 16
    .lba_start:         resb 8
    .lba_end:           resb 8
    .attributes:        resb 8
    .name:              resb 72     ; Note: Never hardcode this length when reading strings. Always use to the end of part_entry_size
endstruc