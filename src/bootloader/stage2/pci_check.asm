bits 16

%include "common.inc"

global pci_check_v2_installed

section .text

pci_check_v2_installed:
    mov ax, 0xB101
    int 0x1A

    ; Check the the function was supported
    cmp edx, 0x20494350
    je .supported

    ; If the function was not supported then assume not installed
    mov ah, 0xFF

.supported:
    test ah, ah
    jnz .not_installed
    not ah
    ret

.not_installed:
    xor al, al
    ret