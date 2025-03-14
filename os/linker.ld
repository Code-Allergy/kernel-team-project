SECTIONS {
    . = 0xA0000000;
    
    .header ALIGN(4K) : {
        PROVIDE(kernel_header = .); /* Define a global label */
        LONG(0x1B1B1B1B)   /* Magic number */
        LONG(ADDR(.text))  /* Pointer to .text section */
        LONG(ADDR(.data))  /* Pointer to .data section */
        LONG(ADDR(.bss))   /* Pointer to .bss section */
        LONG(SIZEOF(.text) + SIZEOF(.rodata) + SIZEOF(.data) + SIZEOF(.bss)) /* Kernel size */
        LONG(ADDR(.text))  /* Entry point (same as .text) */
        LONG(kernel_end)   /* Kernel end */
        LONG(0x0)          /* Reserved */

        /* pad the rest out to 64 bytes */
        QUAD(0x0)
        QUAD(0x0)
        QUAD(0x0)
        QUAD(0x0)
    }
    PROVIDE(kernel_start = .);
    .text : {
        KEEP( *( .text.kmain ) )
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    /* Align the data section to 1M, so we can map it to a different physical section */
    /*.data ALIGN(1M) : { */
    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
    PROVIDE(kernel_end = .);
}
