bits 16

%include "common.inc"

global putc
global puts
global putline
global put_dec
global put_hex
global put_unsigned_double
global hard_error

section .text

    ; 
    ; print a character to the screen
    ; Parameters:
    ;   al: character to print
    ;
    putc:
        ; Save modified registers
        push ax
        push bx

        mov bh, 0x01    ; Page to print on
        mov ah, 0x0E    ; Call interrupt 0x10:0E
        sti
        int 0x10
        cli

        ; Restore modified registers
        pop bx
        pop ax
        ret

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
        cli
        int 0x10
        sti
        jmp .loop       ; Continue to next character
        
    .done:
        ; Restore modified registers
        pop ax
        pop si
        ret

    ;
    ; Print a new line
    ;
    putline:
        push ax
        mov al, CARRIAGE_RETURN
        call putc
        mov al, LINE_FEED
        call putc
        pop ax
        ret

    ;
    ; Print an unsigned number in base 16
    ;   eax: number to print
    ;
    put_hex:
        push ecx
        mov ecx, 16
        call put_unsigned_double
        pop ecx
        ret

    ;
    ; Print an unsigned number in base ten
    ;   eax: number to print
    ;
    put_dec:
        push ecx
        mov ecx, 10
        call put_unsigned_double
        pop ecx
        ret

    ;
    ; Print an unsigned number in a given base
    ;   eax: number to print
    ;   ecx: base
    ;
    put_unsigned_double:
        push eax
        push ecx
        push edx
        push si

        mov si, print_buffer        ; <esi>    Set si to index of next character
        mov [si], byte 0            ;          Store end of string at start of print buffer
        inc si                      ; <esi>
    .convert_loop:
        xor edx, edx                ; <edx>     Clear edx
        div ecx                     ; <eax, edx> divide edx:eax by the base -> eax: quotient, edx: remainder
        add dx, '0'                 ; <edx>     convert remainder to ASCII value
        cmp dx, '9'
        jle .ascii_converted        ;           If greater than 9 then convert to hex digit
        add dx, 'A'-'9' - 1         ; <edx>     10 needs to be converted to A
    .ascii_converted:
        mov [si], dx                ;           store character in print buffer
        inc si                      ; <esi>
        or eax, eax                 ;           if ax not zero repeat loop
        jnz .convert_loop
    .done_convert:
        dec si          ; <si>      Move si back since it was incremented but there was no new character
        std             ;           Reverse direction of string ops to print in reverse order
        call puts       ;           Print the number
        cld             ;           Reset direction of string ops

        pop si
        pop edx
        pop ecx
        pop eax
        ret

    ;
    ; Print an error message and stop the program
    ; Parameters:
    ;   si: Address of error message
    ; Does not return
    ;
    hard_error:
        call puts
        call putline
    .halt:
        jmp .halt

section .bss
    print_buffer: resb 256