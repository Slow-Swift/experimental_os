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

; ELF Structures

struc elf_header
    .magic:             resb    4
    .bitness:           resb    1
    .endian:            resb    1
    .header_version:    resb    1
    .os_abi:            resb    1
    .reserved:          resb    8
    .type:              resb    2
    .instruction_set:   resb    2
    .elf_version:       resb    4
    .entry_pos:         resb    4
    .prog_table_pos:    resb    4
    .seg_table_pos:     resb    4
    .flags:             resb    4
    .header_size:       resb    2
    .prog_table_entry_size:     resb    2
    .prog_table_entry_count:    resb    2
    .seg_table_entry_size:      resb    2
    .seg_table_entry_count:     resb    2
    .section_names_index:       resb    2
endstruc

struc elf_program_header
    .type:          resb    4
    .offset:        resb    4
    .virtual_addr:  resb    4
    .reserved:      resb    4
    .file_size:     resb    4
    .memory_size:   resb    4
    .flags:         resb    4
    .alignment:     resb    4
endstruc