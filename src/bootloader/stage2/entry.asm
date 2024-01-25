bits 16

%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

section .entry

    global entry
    entry:
        cli
        mov si, hello_world_msg
        call puts
        jmp halt

    ; 
    ; print a string to the screen
    ; Parameters:
    ;   si: address of string
    ;
    puts:
        ; Save modified registers
        push si
        push ax
        
    .loop:
        lodsb       ; Load next character into al
        or al, al   ; Exit if it is null
        jz .done

        mov ah, 0x0E    ; Call interrupt 0x10 - 0x0E to print al
        int 0x10
        jmp .loop       ; Continue to next character
        
    .done:
        ; Restore modified registers
        pop ax
        pop si
        ret

    halt:
        jmp halt

hello_world_msg: db "Hello World From Stage 2", ENDL, 0