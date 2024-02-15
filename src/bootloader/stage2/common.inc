%define CARRIAGE_RETURN 0x0D
%define LINE_FEED       0x0A
%define ENDL CARRIAGE_RETURN, LINE_FEED
%define SECTOR_SIZE 512

%define boot_data_addr  0x600

struc boot_data
    .next_availiable_mem:    resb 4
    .memory_map_addr:        resb 4
    .memory_map_count:       resb 4
    .memory_map_struct_size: resb 1
    .disk_code:              resb 1
                             resb 2
endstruc