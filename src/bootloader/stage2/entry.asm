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

extern enable_a20

extern disk_initialize
extern gpt_initialize
extern fat_initialize

extern fat_open_root_directory
extern fat_find_file_in_directory
extern fat_open_file
extern fat_copy_sector

extern load_gdt
extern enter_unreal_mode

extern kl_load_kernel

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
        mov si, disk_initialized_msg
        call puts

        call gpt_initialize   
        mov si, gpt_initialized_msg
        call puts

        call fat_initialize
        mov si, fat_initialized_msg
        call puts

        call load_gdt
        call enter_unreal_mode

        mov si, unreal_mode_msg
        call puts

        call kl_load_kernel

    halt:
        jmp halt

section .rodata
    stage_2_loaded_msg: db "Stage 2 Starting...", ENDL, 0
    disk_initialized_msg:   db "Initalized Disk...", ENDL, 0
    gpt_initialized_msg:   db "Initalized GPT...", ENDL, 0
    fat_initialized_msg:   db "Initalized FAT...", ENDL, 0
    unreal_mode_msg:   db "Unreal Mode Enabled...", ENDL, 0
    boot_folder:    db "BOOT       ", 0
    kernel_name:    db "KERNEL  ELF", 0
    filename:       db "TEST    TXT", 0

section .data
    disk_code: db 0

section .bss
    disk_test_buffer: resb 512