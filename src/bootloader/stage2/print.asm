section .text

global puts
global putc
global putuid

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
        int 0x10

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
        int 0x10
        jmp .loop       ; Continue to next character
        
    .done:
        ; Restore modified registers
        pop ax
        pop si
        ret

    ; 
    ; print a character to the screen
    ; Parameters:
    ;   eax: character to print
    ;   ecx: base
    ;
    putuid:
        push eax
        push edx
        push bx

    .print_loop:
        xor edx, edx    ; <edx>         Clear upper part of dividend
        div ecx         ; <eax, edx>    Divide eax by base: eax (quotient), edx (remainder)
        push eax        ;               Save quotient
        mov al, dl      ; <eax>         Copy remainder (digit) to al
        add al, '0'     ; <eax>         Convert 0x0 to '0'
        cmp al, '9'     ;               Check if greater than a digit 
        jle .digit_converted    ;       
        add al, 'A' - '0'   ; <eax>     If greater than digits then need to convert to hex digit
    .digit_converted:
        call putc
        pop eax         ; <eax>
        jnz .print_loop
    .end:
        pop bx
        pop edx
        pop eax
        ret

section .bss
    print_buffer: resb 256