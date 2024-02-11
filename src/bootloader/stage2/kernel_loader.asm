bits 16

extern puts
extern hard_error

extern fat_open_root_directory
extern fat_find_file_in_directory
extern fat_open_file

global kl_load_kernel

section .text:

    kl_load_kernel:
        call open_kernel
        ret

    open_kernel:
        call fat_open_root_directory        ;           Open the root directory
        mov si, boot_folder                 ; <esi>     Will look for boot directory
        call fat_find_file_in_directory     ; <eax, ebx>    Find the boot directory

        test eax, eax                       ;           Check cluster not zero (file not found)
        je .boot_folder_not_found           ;           If cluster zero then file was not found
        not bl                              ; <ebx>     If bl is not -1 then it was not directory
        test bl, bl
        jnz .boot_folder_not_found          ;           Error if it was not a directory

        not bl
        call fat_open_file                  ;           Open the root directory
        mov si, kernel_name                 ; <esi>     Search for the kernel name
        call fat_find_file_in_directory     ; <eax, ebx>    Find the kernel file
        test eax, eax                       ;           Check cluster not zero (file not found)
        je .kernel_not_found                ;           If cluster zero then error since file was not found

        mov si, kernel_found_msg
        call puts

        call fat_open_file      ; Open the kernel file

        ret
    .boot_folder_not_found:
        mov si, boot_folder_not_found_err
        call hard_error
    .kernel_not_found:
        mov si, kernel_not_found_err
        call hard_error

section .rodata:
    boot_folder_not_found_err:  db "KERNEL_LOAD_ERROR: Could not find boot folder", 0
    kernel_not_found_err:  db "KERNEL_LOAD_ERROR: Could not find kernel file", 0
    boot_folder: db "BOOT       ", 0
    kernel_name: db "KERNEL  ELF", 0
    kernel_found_msg: db "Found Kernel", 0