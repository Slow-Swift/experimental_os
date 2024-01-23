ENTRY(start)
OUTPUT_FORMAT("binary")
phys = 0x7C00;

SECTIONS
{
    . = phys;

    .entry              : { __entry_start = .;
                            *(.entry)
                        }
    .text               : { *(.text)        }
    .data               : { *(.data)        }
    .rodata             : { *(.rodata)      }
    .pmbr        0x7DBE : { SHORT(0x00)        }
    .bios_footer 0x7DFE : { SHORT(0xAA55)   }
    .bss         0x7E00 : { *(.bss)         }
}