bits 16

; Character Codes
%define CARRIAGE_RETURN 0x0D
%define LINE_FEED 0x0A
%define BACKSPACE 0x08
%define ENDL 0x0D, 0x0A

section .entry
    global start
    start:
        ; Setup segments all to 0. Stack segment to 0x7C0:0 (grows downward)
        mov ax, 0x7C0
        mov ss, ax
        xor ax, ax
        mov ds, ax
        mov es, ax
        mov sp, ax

    .main_loop:
        call read_line

        mov al, [charBuffer]    ; Copy the first character of the line into al
    .switch_plus:
        cmp al, '+'             ; Check if first char is '+'
        jne .switch_dash        ; If not move to next check
        call op_add             ; Call add
        jmp .finish_op
    .switch_dash:
        cmp al, '-'             ; Check if first char is '-'
        jne .switch_star     ; If not move to next check
        call op_sub             ; Call sub
        jmp .finish_op
    .switch_star:
        cmp al, '*'
        jne .switch_slash
        call op_mul
        jmp .finish_op
    .switch_slash:
        cmp al, '/'
        jne .switch_default
        call op_div
        jmp .finish_op
    .finish_op:
        or ax, ax
        jnz .switch_end
        pop ax
        call printNum
        call printLine
        push ax
        jmp .switch_end
    .switch_default:
        call convert_to_num           ; If it was not an operation push the number
        push ax
    .switch_end:
        jmp .main_loop

    hlt:
        jmp hlt

section .text

    ;
    ; si: address of string
    ;
    puts:
        ; Save modified registers
        push si
        push ax
        
    .loop:
        lodsb       ; Load next character
        or al, al   ; Exit if it is null
        jz .done

        mov ah, 0x0E    ; Call interrupt 0x10 - 0x0E to print al
        int 0x10
        jmp .loop   ; Continue to next character
        
    .done:
        ; Restore modified registers
        pop ax
        pop si
        ret

    ;
    ; Output a single character
    ; al: char to print
    ;
    putc:
        ; Save modified registers
        push ax

        ; Call interrupt 0x10/0E to print al
        mov bh, 0x0     ; Page to print on
        mov ah, 0x0E    ; Subfunction to call (0E: print character)
        int 0x10
        pop ax
        ret

    ;
    ; Print a number in decimal
    ; ax: number to print
    ;
    printNum:
        push ax
        push cx
        push dx
        push si

        mov [printBuffer], byte 0   ; Store end of string at printBuffer 0
        mov si, printBuffer         ; Set si to index of next character
        inc si
        mov cx, 10                  ; base
    .convert_loop:
        xor dx, dx      ; Clear dx
        div cx          ; divide dx:ax by 10 -> ax: quotient, dx: remainder
        add dx, 0x30    ; convert remainder to ASCII value
        mov [si], dx    ; store character in print buffer
        inc si
        or ax, ax       ; if ax not zero repeat loop
        jnz .convert_loop
    .done_convert:
        dec si          ; Move si back since it was incremented but there was no new character
        std             ; Reverse direction of string ops to print in reverse order
        call puts       ; Print the number
        cld             ; Reset direction of string ops

        pop si
        pop dx
        pop cx
        pop ax
        ret

    ;
    ; Print a new line
    ;
    printLine:
        push si
        mov si, new_line
        call puts
        pop si
        ret
    ;
    ; Read line
    ;
    read_line:
        ; print '> ' before reading line
        mov al, '>'
        call putc
        mov al, ' '
        call putc

        mov si, 0           ; Index into line
    .typing_loop:
        mov ah, 0x00        ; Call interrupt 0x16 - 0x00 to get char
        int 0x16
        or al, al           ; Check for char not null
        jz .typing_loop     ; If it is null restart loop
        call putc           ; Print the character

        cmp al, CARRIAGE_RETURN     ; Check for carriage return and exit if typed
        je .end_line

        cmp al, BACKSPACE   ; Check for backspace
        jne .normal_char   
        mov al, ' '         ; Overwrite current char with ' '
        call putc           
        mov al, BACKSPACE   ; Move cursor back
        call putc           
        dec si              ; Reduce string length
        jmp .typing_loop

    .normal_char:
        mov [si + charBuffer], al   ; Add character to line
        inc si
        jmp .typing_loop
    .end_line:
        mov [si + charBuffer], byte 0    ; Add null terminator to line
        mov al, LINE_FEED
        call putc
        ret



    ; 
    ; Add the last two items on the stack
    ;
    op_add:
        cmp sp, -6
        jle .ok
            push si
            mov si, not_enough_stack_elements_error
            call puts
            pop si
            mov ax, 1
            ret
    .ok:
        pop cx     ; Store return address
        pop bx      ; Get operands
        pop ax
        add ax, bx  ; Perform operation
        push ax     
        push cx    ; Restore return address
        xor ax, ax
        ret

    ;
    ; Subtract the last two items on the stack
    ;
    op_sub:
        cmp sp, -6
        jle .ok
            push si
            mov si, not_enough_stack_elements_error
            call puts
            pop si
            mov ax, 1
            ret
    .ok:
        pop cx     ; Store return address
        pop bx     ; Get operands
        pop ax
        sub ax, bx  ; Perform operation
        push ax     
        push cx    ; Restore return address
        xor ax, ax
        ret

    ;
    ; Subtract the last two items on the stack
    ;
    op_mul:
        cmp sp, -6
        jle .ok
            push si
            mov si, not_enough_stack_elements_error
            call puts
            pop si
            mov ax, 1
            ret
    .ok:
        pop cx     ; Store return address
        pop bx     ; Get operands
        pop ax
        mul bx  ; Perform operation
        push ax     
        push cx    ; Restore return address
        xor ax, ax
        ret

    ;
    ; Subtract the last two items on the stack
    ;
    op_div:
        cmp sp, -6
        jle .num_ok
            push si
            mov si, not_enough_stack_elements_error
            call puts
            pop si
            mov ax, 1
            ret
    .num_ok:
        pop cx     ; Store return address
        pop bx     ; Get operands
        or bx, bx
        jnz .ok
    .if_zero:
        push si
        mov si, div_zero_error
        call puts
        pop si
        push cx
        mov ax, 1
        ret
    .ok:
        pop ax
        div bx     ; Perform operation
        push ax     
        push cx    ; Restore return address
        xor ax, ax
        ret

    ;
    ; Convert the line to an integer
    ; ax: converted value
    ;
    convert_to_num:
        mov si, charBuffer  ; Pointer to next char
        xor ax, ax           ; Converted value
        mov cx, 10          ; Base
    .loop:
        push ax             ; Save ax
        lodsb               ; Load next character
        mov bl, al          ; Move character into bx
        pop ax              ; Restore ax
        or bl, bl           ; If it is null we have reached the end
        jz .done

        cmp bl, 0x30        ; If the character is not a digit report an error
        jl .error
        cmp bl, 0x39
        jg .error

        sub bl, 0x30        ; Convert ASCII code to 0-9
        mul cx              ; Shift converted value up by 1
        add ax, bx          ; Add new digit
        jmp .loop
    .error:
        mov si, nan_error
        call puts
    .done:
        ret

section .rodata
    nan_error: db 'Not a valid num', ENDL, 0
    div_zero_error: db 'Cannot divide by 0', ENDL, 0
    new_line: db ENDL, 0
    not_enough_stack_elements_error: db 'Not enough', ENDL, 0

section .bss
    printBuffer: resb 256
    charBuffer: resb 256
