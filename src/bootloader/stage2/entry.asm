bits 16

%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

%include "gpt_structs.inc"

extern puts
extern putc
extern put_hex
extern putline

extern disk_initialize
extern disk_seek_abs
extern disk_copy_bytes

extern enable_a20

extern gpt_initialize
extern gpt_get_partition

section .entry

    global entry
    entry:
        cli

        ; Save disk code
        mov [disk_code], dl

        ; Initialize all registers to zero
        xor eax, eax
        mov ebx, eax
        mov ecx, eax
        mov edx, eax
        mov esi, eax
        mov edi, eax
        mov es, ax
        mov ds, ax
        
        ; Initialize stack
        mov ss, ax
        mov sp, 0x1000
        call main

section .text
    main:
        ; Print a starting message
        mov si, stage_2_loaded_msg
        call puts

        ; Enable the A20 Gate if it is disabled
        call enable_a20

        ; Initialize the disk
        mov dl, [disk_code]
        call disk_initialize

        call gpt_initialize   

        mov eax, 0
        mov di, test_partition_entry
        call gpt_get_partition

        mov eax, [test_partition_entry + partition_entry.lba_start]
        call put_hex

    halt:
        jmp halt

section .rodata
    stage_2_loaded_msg: db "Stage 2 Starting...", ENDL, 0
    disk_read_msg: db "Disk Read...", ENDL, 0
    used_partition_found: db "Used Partition Found", ENDL, 0
    unused_partition_found: db "Used Partition Found", ENDL, 0


section .data
    disk_code: db 0

section .bss
    test_partition_entry: resb partition_entry_size