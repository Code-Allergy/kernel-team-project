// we do not need this, not ELF
// ENTRY(main)

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

SECTIONS
{
    /* .text section */
    .text : {
        // first thing we reach when bootloader jumps
		KEEP( *( .text.main) )
		*( .text* )
	} > DRAM
    . = ALIGN(4);  /* Ensure proper alignment for the next section */
}
