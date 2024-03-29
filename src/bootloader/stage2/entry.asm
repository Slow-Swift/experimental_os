bits 16

%include "common.inc"
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
extern enter_protected_mode

extern kl_load_kernel

extern detect_mem

extern pci_check_v2_installed

; Used for debugging
global prog_end

section .entry

    global entry
    entry:
        cli

        ; Save disk code
        mov dword [boot_data_addr + boot_data.next_availiable_mem], boot_data_size + boot_data_addr
        mov [boot_data_addr + boot_data.disk_code], dl

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
        mov dl, [boot_data_addr + boot_data.disk_code]
        call disk_initialize
        mov si, disk_initialized_msg
        call puts

        call gpt_initialize   
        mov si, gpt_initialized_msg
        call puts

        call fat_initialize
        mov si, fat_initialized_msg
        call puts

        call pci_check_v2_installed
        mov [boot_data_addr + boot_data.pci_v2_installed], ah
        mov [boot_data_addr + boot_data.pci_characteristics], al

        call detect_mem

        call load_gdt
        call enter_unreal_mode

        mov si, unreal_mode_msg
        call puts

        call kl_load_kernel

        mov ecx, boot_data_addr
        call enter_protected_mode

    prog_end:
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

section .bss
    disk_test_buffer: resb 512