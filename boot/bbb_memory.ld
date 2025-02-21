ENTRY(__init)

MEMORY
{
    SRAM (rwx)  : ORIGIN = 0x402F0400, LENGTH = __BBB_LD_SRAM_END - ORIGIN(SRAM)
    L3_OCMCO_RAM (rwx)  : ORIGIN = 0x40300000, LENGTH = __BBB_LD_L3_OCMC0_END - ORIGIN(L3_OCMCO_RAM)     /* 64KB */
    DRAM (rwx)  : ORIGIN = 0x80000000, LENGTH = 512M    /* 512MB */
}

__BBB_LD_SRAM_END = 0x402FFFFF;
__BBB_LD_L3_OCMC0_END = 0x4030FFFF;
__BBB_DRAM_BEGIN = ORIGIN(DRAM);
__BBB_DRAM_END = __BBB_DRAM_BEGIN + 512M;



/* ROM bootloader just loads a single contiguous segment, so make the ELF
 * program headers do the same 
 * ?????
 */
PHDRS {
	image PT_LOAD;
}

SECTIONS
{
    /* .text section */
    .text : {
		KEEP( *( .text._start ) )
		*( .text* )
	} :image
    . = ALIGN(4);  /* Ensure proper alignment for the next section */

    /* .rodata section */
    .rodata : {
		*( .rodata* )
    } :image
    . = ALIGN(4);  /* Ensure proper alignment for the next section */

    /* .data section */
    .data : {
		*( .data* )
	} :image
	. = ALIGN(4);

	.bss : {
	    __BssStart = .;
	    *( .bss* )
	    __BssEnd = .;
	} > SRAM

	/* .vectors : { */
	/* } */

    . = ALIGN(4);
    __StackEnd = .;
    __StackStart = __BBB_LD_SRAM_END - 0xf;

    /* Shared uninitialized RAM */
	.shared (NOLOAD) : { *( .shared .shared.* ) } >L3_OCMCO_RAM :NONE

	/* Private uninitialized RAM */
	.noinit (NOLOAD) : { *( .noinit .noinit.* ) } >SRAM :NONE

    /* Catch any unknown sections and yell loudly about them */
	.orphan : { INPUT_SECTION_FLAGS (SHF_ALLOC) *(*) } :NONE
	ASSERT( SIZEOF(.orphan) == 0, "unassigned sections!" )
}
