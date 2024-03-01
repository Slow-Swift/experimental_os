error_code_isrs = [8, 10, 11, 12, 13, 14, 17, 21, 29, 30]

c_filepath = "src/kernel/arch/i686/isrs_gen.c"
inc_filepath = "src/kernel/arch/i686/isrs_gen.inc"

with open(c_filepath, 'w') as c_file:
    c_file.write('//*  Auto Generated by generate_isrs.py  *//"\n')
    c_file.write('#include "idt.h"\n')
    c_file.write('#include "gdt.h"\n')
    c_file.write('\n')

    for i in range(256):
        c_file.write(f'void __attribute__((cdecl)) i686_isr{i}();\n')

    c_file.write('\n')
    c_file.write('void isr_initialize_gates() {\n')

    for i in range(256):
        c_file.write(f'    idt_set_gate({i}, i686_isr{i}, GDT_CODE_SEGMENT, IDT_FLAG_RING_0 | IDT_FLAG_GATE_32_BIT_INT);\n')
    c_file.write("}\n")

with open(inc_filepath, 'w') as inc_file:
    inc_file.write(';*  Auto Generated by generate_isrs.py"\n')
    inc_file.write('\n')

    for i in range(256):
        if i in error_code_isrs:
            inc_file.write(f"ISR_ERRORCODE {i}\n")
        else:
            inc_file.write(f"ISR_NOERRORCODE {i}\n")