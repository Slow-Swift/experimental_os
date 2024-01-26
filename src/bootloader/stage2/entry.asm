bits 16

%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

extern puts
extern putc
extern putuid

section .entry

    global entry
    entry:
        cli
        mov si, stage_2_loaded_msg
        call puts

    halt:
        jmp halt

section .rodata
    stage_2_loaded_msg: db "Stage 2 Starting...", ENDL, 0