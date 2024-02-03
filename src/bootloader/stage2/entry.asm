bits 16

%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

extern puts
extern putc
extern put_hex
extern putline

extern disk_initialize
extern disk_seek_abs
extern disk_copy_bytes

extern enable_a20

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

        ; Print a starting message
        mov si, stage_2_loaded_msg
        call puts

        ; Enable the A20 Gate if it is disabled
        call enable_a20

        ; Initialize the disk
        mov dl, [disk_code]
        call disk_initialize

        ; Seek 0x1:0
        mov eax, 0x1
        mov ebx, 0
        call disk_seek_abs

        mov si, disk_read_msg
        call puts

        mov di, test_disk_data
        mov ebx, 20
        call disk_copy_bytes
        mov eax, [test_disk_data]
        call put_hex


    halt:
        jmp halt

section .rodata
    stage_2_loaded_msg: db "Stage 2 Starting...", ENDL, 0
    disk_read_msg: db "Disk Read..."

section .data
    disk_code: db 0

section .bss
    test_disk_data: resb 20