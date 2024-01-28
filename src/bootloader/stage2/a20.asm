bits 16

%include "common.inc"

extern puts

global enable_a20

section .text

    ;
    ; Enable the A20 gate if it was disabled
    ;
    enable_a20:
        call check_a20
        or ax, ax
        jnz .bios_a20
        mov si, a20_enabled
        call puts
        ret
    
    ; ! Not actually sure that anything past here works since the a20 gate is already enabled with qemu
    .bios_a20:
        mov si, a20_disabled
        call puts

        call enable_a20_bios    ; Enable the A20 using the bios
        cmp ah, ah              ; Check for success
        jnz .keyboard_a20       ; If not success (ah != 0) then try with the keyboard
        call check_a20          ; Verify success
        or ax, ax
        jnz .keyboard_a20
    
    .keyboard_a20:
        call enable_a20_keyboard
        call check_a20          ; Verify success
        or ax, ax
        jnz .still_disabled
        mov si, a20_enabled
        call puts
        ret

    .still_disabled:
        mov si, a20_disabled
        call puts
        ret

    ;
    ; Enable the A20 Gate using the BIOS
    ; 
    ; Returns:
    ;   ah: 0 on success
    ;   ah: Nonzero on failure
    ;
    enable_a20_bios:
        mov ax, 0x2403              ; Call interrupt 0x15:2403 (Query A20 Gate Support)
        int 0x15
        jnc .int15_not_supported    ; If the carry flag is set then BIOS a20 is not supported
        or ah, ah
        jnz .int15_not_supported    ; If ah is not zero then BIOS a20 is not supported

        mov ax, 0x20                ; Call interrupt 0x15:2401 (Enable A20 Gate)
        int 0x15
        jc .int15_failed            ; Carry flag set on failure
        or ah, ah
        jc .int15_failed            ; AH contains nonzero status on failure
        ret

    .int15_not_supported:
        mov si, a20_bios_not_supported
        call puts
        ret

    .int15_failed:
        mov si, a20_bios_failed
        call puts
        ret

    enable_a20_keyboard:
        cli

        call .wait_io1
        mov al, 0xad
        out 0x64, al

        call .wait_io1
        mov al, 0xd0
        out 0x64, al

        call .wait_io2
        in al, 0x60
        push eax

        call .wait_io1
        mov al, 0xD1
        out 0x64, al

        call .wait_io1
        pop eax
        or al, 2        ; enable a20 bit
        out 0x60, al

        call .wait_io1
        mov al, 0xae
        out 0x64, al

        call .wait_io1
        sti
        ret

        .wait_io1:
            in al, 0x64
            test al, 2      ; Wait for second bit to be off
            jnz .wait_io1
            ret

        .wait_io2:
            in al, 0x64
            test al, 1      ; Wait for first bit to be off
            jnz .wait_io1
            ret

    check_a20:
        cli
        push ds
        push es
        push si
        push di

        xor ax, ax  ; <ax>      Clear ax    
        mov ds, ax  ; <ds>      Clear es
        mov si, a20_check

        not ax      ; <ax>      ax = 0xFFFF
        mov es, ax  ; <es>      ds = 0xFFFF
        mov di, 10
        add di, a20_check   ;   es:di = 0xFFFF:(a20_check+10) = a20_check + 1M

        mov [es:di], ax     ; Move 0xFFFF into the a20_check + 1M

        mov ax, [ds:si]     ; Check what is in the a20_check
                            ; Should be 0 if a20 gate is enabled, 0xFFFF if not

        pop di
        pop si
        pop ds
        pop es
        sti
        ret

section .rodata:
    a20_enabled: db "A20 Gate Enabled", ENDL, 0
    a20_disabled: db "A20 Gate Disabled", ENDL, 0
    a20_bios_not_supported: db "BIOS A20 Not Supported", ENDL, 0
    a20_bios_failed: db "BIOS A20 Failed", ENDL, 0

section .bss
    a20_check: resb 1   ; This should be cleared to zero