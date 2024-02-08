struc ebpb_struct
    ; bios parameter block
    .short_jump:            resb 3
    .oem_identifier:        resb 8
    .bytes_per_sector:      resb 2
    .sectors_per_cluster:   resb 1
    .reserved_sectors:      resb 2
    .fat_count:             resb 1
    .root_dir_entries:      resb 2
    .total_sectors:         resb 2
    .media_descriptor_type: resb 1
    .sectors_per_fat_12_16: resb 2
    .sectors_per_track:     resb 2
    .head_count:            resb 2
    .hidden_sector_count:   resb 4
    .large_sector_count:    resb 4

    ; FAT32 data
    .sectors_per_fat:       resb 4
    .flags:                 resb 2
    .fat_version:           resb 2
    .root_cluster:          resb 4
    .fs_info_sector:        resb 2
    .backup_boot_sector:    resb 2
    .reserved:              resb 12
    .drive_number:          resb 1
    .flags_nt:              resb 1
    .signature:             resb 1
    .volume_id:             resb 4
    .volume_label:          resb 11
    .system_identifier:     resb 8
endstruc

struc fs_info_struc
    .signature:             resb 4
    .reserved:              resb 480
    .signature_2:           resb 4
    .last_free_cluster:     resb 4
    .free_cluster:          resb 4
endstruc

struc dir_entry_struct
    .name:                  resb 11
    .attributes:            resb 1
    .reserved:              resb 1
    .created_time_tenths:   resb 1
    .created_time:          resb 2
    .created_date:          resb 2
    .accessed_date:         resb 2
    .first_cluster_high:   resb 2
    .modified_time:         resb 2
    .modified_date:         resb 2
    .first_cluster_low:     resb 2
    .size:                  resb 4
endstruc

struc long_filename
    .order:         resb 1
    .chars0_4:      resb 10
    .attribute:     resb 1
    .checksum:      resb 1
    .chars5_10:     resb 12
    .reserved:      resb 2
    .chars11_12:    resb 4
endstruc