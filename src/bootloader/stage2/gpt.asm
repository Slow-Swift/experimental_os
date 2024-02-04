bits 16

%include "gpt_structs.inc"

extern disk_seek_abs
extern disk_copy_bytes

extern hard_error

global gpt_initialize
global gpt_get_partition

section .text

    ; 
    ; Initialize the GPT
    ;
    ; Volatile:
    ;   disk seek position
    gpt_initialize:
        push eax
        push ebx
        push di

        ; Seek to first sector
        mov eax, 1              ; <eax>     Seek to LBA 1
        mov ebx, 0              ; <ebx>     Seek to offset 0 from LBA
        call disk_seek_abs      ;           Perform the seek
        
        ; Load the gpt header   
        mov di, gpt_header                      ; <edi>     Copy bytes to the gpt_header             
        mov ebx, partition_table_header_size    ; <ebx>     Need to copy the entire gpt header
        call disk_copy_bytes                    ;           Perform the copy

        ; Check that the GPT header magic = "EFI PART"
        cmp dword [gpt_header + partition_table_header.magic], "EFI "       ; Check the first 4 bytes
        jne .invalid_hdr                                
        cmp dword [gpt_header + partition_table_header.magic + 4], "PART"   ; Check the second four bytes
        jne .invalid_hdr
            
        ; Header ok so return
        pop di
        pop ebx
        pop eax
        ret

    .invalid_hdr:
        mov si, error_invalid_header
        call hard_error

    ;
    ; Get a parition
    ; Parameters:
    ;   eax: partition number
    ;   es:di: address to store parition
    ; Returns:
    ;   al: 1 if partition is used (nonzero type guid), 0 otherwise
    ;
    gpt_get_partition:
        push eax
        push ebx
        push edx
        
        mov edx, [gpt_header + partition_table_header.part_entry_size]  ; <edx>     Determine parition entry size
        mul edx     ; <eax, edx>    Determine offset of parition header. It is asssumed that this fits in eax

        ; Seek to the position of the partition entry
        mov ebx, eax    ; <ebx>     Store partition entry offset in ebx
        mov eax, [gpt_header + partition_table_header.part_array_lba]   ; <eax>     Determine LBA of first parition entry
        call disk_seek_abs      ; Seek to the partition entry

        ; Read the parititon entry
        mov ebx, partition_entry_size
        call disk_copy_bytes

        ; If any of the guid bits are non-zero then return 1
        xor bx, bx      ; <ebx>     Clear bx
        inc bx          ; <ebx>     bx = 1
        xor eax, eax    ; <eax>     Clear eax

        ; Or each part of the guid with eax so eax will be nonzero if anything is nonzero
        or eax, [di + partition_entry.type_guid]
        or eax, [di + partition_entry.type_guid + 4]
        or eax, [di + partition_entry.type_guid + 8]
        or eax, [di + partition_entry.type_guid + 12]
        cmovnz ax, bx

        pop edx
        pop ebx
        pop eax
        ret

section .rodata
    error_invalid_header: db "GPT ERROR: Invalid EFI Header"

section .bss
    gpt_header: resb partition_table_header_size