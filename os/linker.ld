SECTIONS {
    /* . = 0xA0000000; */

    /* for now, we can just start the kernel in physical memory */
    . = 0x80000000;


    .header ALIGN(4K) : {
        PROVIDE(kernel_header = .); /* Define a global label */
        LONG(0x1B1B1B1B)   /* Magic number */
        LONG(ADDR(.text))  /* Pointer to .text section */
        LONG(ADDR(.data))  /* Pointer to .data section */
        LONG(ADDR(.bss))   /* Pointer to .bss section */
        LONG(SIZEOF(.text) + SIZEOF(.data) + SIZEOF(.bss)) /* Kernel size */
        LONG(ADDR(.text))  /* Entry point (same as .text) */
        LONG(0x0)          /* Reserved */
        LONG(0x0)          /* Reserved */

        /* pad the rest out to 64 bytes */
        QUAD(0x0)
        QUAD(0x0)
        QUAD(0x0)
        QUAD(0x0)
    }

    .text : {
        KEEP( *( .text.kmain ) )
        *(.text)
    }

    .rodata ALIGN(4K) : {
        *(.rodata)
    }

    .data ALIGN(4K) : {
        *(.data)
    }

    .bss ALIGN(4K) : {
        *(.bss)
    }
}
