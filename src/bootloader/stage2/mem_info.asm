[bits 16]

%include "common.inc"
%include "mem_info_structs.inc"

extern puts
extern put_hex
extern putline

global detect_mem

section .text

    detect_mem:
        pushad

        mov si, mem_detect_msg
        call puts

        xor ebx, ebx    ; Start at beginning of map
        mov di, [boot_data_addr + boot_data.next_availiable_mem]    ; Load address to store memory map
        mov [boot_data_addr + boot_data.memory_map_addr], di        ; Store address of memory map
 
    .detect_loop:
        mov edx, 0x534D4150  ; <edx> Load magic into edx
        xor eax, eax    ; <eax> Clear eax
        mov ax, 0xE820  ; <eax> Load interrupt function into eax
        xor ecx, ecx    ; Clear ecx
        mov cx, mem_area_struct_size    ; <ecx> Load availiable size into ecx
        int 0x15

        jc .done        ; If carry set then error or done
        test ebx, ebx   ; If ebx 0 then done
        jz .done

        mov [boot_data_addr + boot_data.memory_map_struct_size], cl   ; Save size of struct

        mov si, start_chars
        call puts

        ; Print the base address
        mov eax, [di + mem_area_struct.base_addr + 4]
        call put_hex
        mov eax, [di + mem_area_struct.base_addr]
        call put_hex
        mov si, middle_chars
        call puts

        ; Print the length
        mov eax, [di + mem_area_struct.length + 4]
        call put_hex
        mov eax, [di + mem_area_struct.length]
        call put_hex
        mov si, middle_chars
        call puts

        ; Print the type
        mov eax, [di + mem_area_struct.type]
        call put_hex
        mov si, middle_chars
        call puts

        ; Print the attributes
        mov eax, [di + mem_area_struct.extended_attributes]
        call put_hex
        mov si, end_chars
        call puts

        add di, mem_area_struct_size    ; <ecx> Move to next struct
        jmp .detect_loop

    .done:
        xor eax, eax    ; <eax> Clear eax
        xor edx, edx    ; <edx> Clear edx
        mov ax, di      ; <eax> Copy address of end of memory map
        sub ax, [boot_data_addr + boot_data.next_availiable_mem]   ; Determine size of entire memory map
        mov bx, mem_area_struct_size    ; <ebx> Get size of mem struct
        div bx          ; <eax, edx>    ax = memory struct count
        mov [boot_data_addr + boot_data.memory_map_count], ax      ; Save the number of memory structures
        mov [boot_data_addr + boot_data.next_availiable_mem], di   ; Save the addess of next memory

        popad
        ret

section .rodata
    mem_detect_msg: db "Detecting Memory:", ENDL, 
                    db "  [BASE] [LENGTH] [TYPE] [Attributes]", ENDL, 0
                    
    start_chars:    db "  [", 0
    middle_chars:   db "] [", 0
    end_chars:      db "]", ENDL, 0