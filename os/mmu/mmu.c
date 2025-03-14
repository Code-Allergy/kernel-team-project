// Memory management
#include "uart.h"
#include <utils.h>
#include <types.h>
#include <mmu.h>
#include <mem.h>

#define L1_SECTION_MASK 0xFFF00000

#define GET_L1_INDEX(x) ((x) >> 20)
#define GET_L2_INDEX(x) (((x) >> 12) & 0xFF)


/* Clear the bootloader page tables (they will have DRAM stock pattern on them) */
static inline void clear_boot_tables(void) {
    int32_t i;
    uint32_t *l1_base = (uint32_t *)MEM_BOOT_PAGE_TABLE_BASE;
    for (i = 0; i < 4096; i++) {
        l1_base[i] = 0;
    }
}

/* Just use domain 0 for kernel, enable memory protection (client mode) */
void mmu_set_domains(void) {
    uint32_t dacr = 0x1;
    __asm__ volatile("mcr p15, 0, %0, c3, c0, 0" : : "r"(dacr));
}

void map_section(volatile uint32_t *l1_base, uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    l1_base[GET_L1_INDEX(vaddr)] = (paddr & L1_SECTION_MASK) | L1_SECTION_DESCRIPTOR | flags;
}

void flush_tlb(void) {
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
}

/* Flush TLB by MVA */
void flush_tlb_entry(uint32_t vaddr) {
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 1" : : "r"(vaddr));
}

/* Flush TLB by asid */
void flush_tlb_asid(uint32_t asid) {
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 2" : : "r"(asid));
}

/* Enable instruction cache */
void i_cache_enable(void) {
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value |= 0x1000;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Enable data cache */
void d_cache_enable(void) {
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value |= 0x4;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Disable instruction cache */
void i_cache_disable(void) {
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value &= ~0x1000;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Disable data cache */
void d_cache_disable(void) {
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value &= ~0x4;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Flush instruction cache */
void flush_i_cache(void) {
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0));
}

/* Flush data cache */
void flush_d_cache(void) {
    __asm__ volatile("mcr p15, 0, %0, c7, c6, 0" : : "r"(0));
}


/* Enable MMU (internal reg flip) */
static void _mmu_enable(void) {
    uint32_t control;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(control));
    control |= 0x1;                             /* Enable MMU */
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(control));
}

void MMU_init(void) {
    uint32_t* l1_tables = (uint32_t*)MEM_BOOT_PAGE_TABLE_BASE;
    uint32_t vaddr, entry;
    uint32_t step = MEM_SECTION_SIZE;
    clear_boot_tables();
    mmu_set_domains();

    /* for now, just map everything 1:1, worry about enabling caching on DRAM later. */
    for (vaddr = 0; vaddr < 0xFFFFFFFF - step; vaddr += step) {
        map_section(l1_tables, vaddr, vaddr, L1_ACCESS_RW_RW);
        // log_l1_pte(l1_tables[GET_L1_INDEX(vaddr)]);
    }
    log_message(LOG_LEVEL_INFO, "Wrote section entries\n");

    /* Also remap only the first 1MB of kernel to first 1MB of DRAM */
    /* We can decide how to determine if we need more static kernel mem */
    /* CAN'T use code pages (RO) unless we split up by L2 page (4K) and
       align code and data sections on a page boundary */
    /* We could align code/data sections to 1MB if we want code write security and NX on data easily */
    map_section(l1_tables, MEM_KERNEL_BASE, MEM_PHYS_BASE, L1_KERNEL_DATA_EXEC_FLAGS);
    set_ttbr0(l1_tables);
    log_message(LOG_LEVEL_INFO, "Loaded L1 tables located at %x into TTBR0\n");
}

void MMU_enable(void) {
    flush_tlb();
    flush_i_cache();
    d_cache_enable();
    i_cache_enable();
    _mmu_enable();
    log_message(LOG_LEVEL_INFO, "MMU Enabled!\n");
}

void MMU_disable(void) {
    mmu_disable();
    d_cache_disable();
    i_cache_disable();
}


uint32_t read_dacr(void) {
    uint32_t dacr;
    __asm__ volatile("mrc p15, 0, %0, c3, c0, 0" : "=r"(dacr));
    return dacr;
}

void set_ttbr0(uint32_t *l1_base) {
    __asm__ volatile(
        "mcr p15, 0, %0, c2, c0, 0 \n" // TTBR0
        "dsb \n"
        "isb \n"
        : : "r"(l1_base)
    );
}

uint32_t read_ttbr0(void) {
    uint32_t ttbr0;
    __asm__ volatile (
        "mrc p15, 0, %0, c2, c0, 0\n"
        : "=r" (ttbr0)
    );
    return ttbr0;
}

/* debug log l1 entry */
void log_l1_pte(uint32_t value) {
    uint32_t section_type = value & 0x3; /* Bits 1:0: Should be `0b10` for a section */
    uint32_t b = (value >> 2) & 1; /* Bit 2: Bufferable */
    uint32_t c = (value >> 3) & 1; /* Bit 3: Cacheable */
    uint32_t xn = (value >> 4) & 1; /* Bit 4: eXecute Never (XN) */
    uint32_t domain = (value >> 5) & 0xF;   /* Bits 8:5: Domain */
    uint32_t ap = (value >> 10) & 0x3;      /* Bits 11:10: AP */
    uint32_t tex = (value >> 12) & 0x7; /* Bits 14:12: TEX */
    uint32_t ap2 = (value >> 15) & 0x1; /* Bits 15: AP2 */
    uint32_t s = (value >> 16) & 1; /* Bit 16: Shareable */
    uint32_t n_g = (value >> 17) & 1; /* Bit 17: Not Global */
    uint32_t supersection = (value >> 18) & 1; /* Bit 18: Supersection flag */
    uint32_t ns = (value >> 19) & 1; /* Bit 19: Non-Secure */
    uint32_t section_base = value & 0xFFF00000; /* Bits 31:20 */

    const char* type_str;
    switch(section_type) {
        case 0x0:
            type_str = "Invalid";
            break;
        case 0x1:
            type_str = "Page";
            break;
        case 0x2:
            type_str = "Section";
            break;
        default:
            type_str = "Reserved";
            break;
    }

    if (supersection != 0) {
        uart_printf("Error: Supersection not supported on hardware, bit should not be set\n");
        return;
    }

    uart_printf("L1PageTableEntry { base: 0x%x, B: %u, C: %u, AP: %u %u, TEX: %u, "
            "Domain: %u, nG: %u, S: %u, XN: %u, NS: %u, type: %s }\n",
            section_base, b, c, ap2, ap, tex, domain, n_g, s, xn, ns, type_str);
}


/* map vaddr addr to phys by section (1MB chunk) entry */
