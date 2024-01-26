bits 16

%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

extern puts
extern putc
extern putline
extern put_dec

section .entry

    global entry
    entry:
        ; Save disk code
        mov [disk_code], dl

        ; Initialize all registers to zero
        xor eax, eax
        mov ebx, eax
        mov ecx, eax
        mov edx, eax
        mov esi, eax
        mov edi, eax

        ; Print a starting message
        mov si, stage_2_loaded_msg
        call puts

        mov eax, 0b100110
        call put_dec
        call putline

    halt:
        jmp halt

section .rodata
    stage_2_loaded_msg: db "Stage 2 Starting...", ENDL, 0

section .bss
    disk_code: db 0