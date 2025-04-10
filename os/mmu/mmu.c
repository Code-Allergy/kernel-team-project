// Memory management
#include "boot.h"
#include "uart.h"
#include <utils.h>
#include <types.h>
#include <mmu.h>
#include <mem.h>

#define L1_SECTION_MASK 0xFFF00000

#define GET_L1_INDEX(x) ((x) >> 20)
#define GET_L2_INDEX(x) (((x) >> 12) & 0xFF)

/* Clear the bootloader page tables (they will have DRAM stock pattern on them)
 */
void clear_boot_tables(void)
{
    int32_t i;
    uint32_t* l1_base = (uint32_t*) MEM_BOOT_PAGE_TABLE_BASE;
    for (i = 0; i < 4096; i++)
    {
        l1_base[i] = 0;
    }
}

/* Just use domain 0 for kernel, enable memory protection */
void MMU_set_domains(void)
{
    uint32_t dacr = 0x55555555;
    __asm__ volatile("mcr p15, 0, %0, c3, c0, 0" : : "r"(dacr));
}

void MMU_map_section(uint32_t* l1_base,
                     uint32_t vaddr,
                     uint32_t paddr,
                     uint32_t flags)
{
    /* verify vaddr and paddr allignment */
    if (vaddr & ~L1_SECTION_MASK || paddr & ~L1_SECTION_MASK)
    {
        log_message(LOG_LEVEL_ERROR,
                    "MMU_map_section: vaddr and paddr must be 1MB aligned\n");
        return;
    }

    l1_base[GET_L1_INDEX(vaddr)] =
        (paddr & L1_SECTION_MASK) | L1_SECTION_DESCRIPTOR | flags;
}

void flush_tlb(void)
{
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 0" : : "r"(0));
}

/* Flush TLB by MVA */
void flush_tlb_entry(uint32_t vaddr)
{
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 1" : : "r"(vaddr));
}

/* Flush TLB by asid */
void flush_tlb_asid(uint32_t asid)
{
    __asm__ volatile("mcr p15, 0, %0, c8, c7, 2" : : "r"(asid));
}

/* Enable instruction cache */
void i_cache_enable(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value |= 0x1000;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Enable data cache */
void d_cache_enable(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value |= 0x4;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Disable instruction cache */
void i_cache_disable(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value &= ~0x1000;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

/* Disable data cache */
void d_cache_disable(void)
{
    uint32_t value;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(value));
    value &= ~0x4;
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(value));
}

void flush_i_cache(void)
{
    __asm__ volatile("mrs r1, cpsr\n"
                     "bic r2, r1, #0x1F\n"
                     "orr r2, r2, #0x1F\n" // Switch to System mode (privileged)
                     "msr cpsr_c, r2\n"

                     "mcr p15, 0, %0, c7, c5, 0\n"

                     "msr cpsr_c, r1\n" // Restore original mode
                     :
                     : "r"(0)
                     : "r1", "r2", "memory");
}

void flush_d_cache(void)
{
    __asm__ volatile("mrs r1, cpsr\n"
                     "bic r2, r1, #0x1F\n"
                     "orr r2, r2, #0x1F\n"
                     "msr cpsr_c, r2\n"

                     "mov r0, #0\n"
                     "mcr p15, 0, r0, c7, c14, 0\n"

                     "msr cpsr_c, r1\n"
                     :
                     :
                     : "r0", "r1", "r2", "memory");
}

/* Enable MMU (internal reg flip) */
static void _mmu_enable(void)
{
    uint32_t control;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(control));
    control |= 0x1; /* Enable MMU */
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(control));
}

static void _mmu_disable(void)
{
    uint32_t control;
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(control));
    control &= ~0x1; /* Disable MMU */
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" : : "r"(control));
}

void mmu_disable(void) { _mmu_disable(); }

static void mmu_map_hardware_pages(void)
{
    uint32_t i;
    uint32_t* l1_tables = (uint32_t*) MEM_BOOT_PAGE_TABLE_BASE;

    /* MAP SDRAM (0x402F_0400) */
    MMU_map_section(l1_tables,
                    0x40200000,
                    0x40200000,
                    L1_ACCESS_RW_NO | L1_CACHEABLE | L1_SHAREABLE);

    /* MAP L3 OCMC0 */
    MMU_map_section(l1_tables,
                    0x40300000,
                    0x40300000,
                    L1_ACCESS_RW_NO | L1_CACHEABLE | L1_SHAREABLE);

    /* MAP L4 WKUP */
    MMU_map_section(l1_tables, 0x44C00000, 0x44C00000, L1_KERNEL_DEVICE_FLAGS);
    MMU_map_section(l1_tables, 0x44D00000, 0x44D00000, L1_KERNEL_DEVICE_FLAGS);
    MMU_map_section(l1_tables, 0x44E00000, 0x44E00000, L1_KERNEL_DEVICE_FLAGS);
    MMU_map_section(l1_tables, 0x44F00000, 0x44F00000, L1_KERNEL_DEVICE_FLAGS);

    /* MAP L4 PER (0x4800_0000, 16MB) */
    for (i = 0; i < 16; i++)
    {
        MMU_map_section(l1_tables,
                        0x48000000 + (i * MEM_SECTION_SIZE),
                        0x48000000 + (i * MEM_SECTION_SIZE),
                        L1_KERNEL_DEVICE_FLAGS);
    }

    /* MAP L4 FAST (0x4A00_0000, 16MB) */
    for (i = 0; i < 16; i++)
    {
        MMU_map_section(l1_tables,
                        0x4A000000 + (i * MEM_SECTION_SIZE),
                        0x4A000000 + (i * MEM_SECTION_SIZE),
                        L1_KERNEL_DEVICE_FLAGS);
    }

    /* MAP EMIF0 (0x4C00_0000, 16MB) */
    for (i = 0; i < 16; i++)
    {
        MMU_map_section(l1_tables,
                        0x4C000000 + (i * MEM_SECTION_SIZE),
                        0x4C000000 + (i * MEM_SECTION_SIZE),
                        L1_KERNEL_DEVICE_FLAGS);
    }

    /* MAP GPMC (0x5000_0000, 16MB) */
    for (i = 0; i < 16; i++)
    {
        MMU_map_section(l1_tables,
                        0x50000000 + (i * MEM_SECTION_SIZE),
                        0x50000000 + (i * MEM_SECTION_SIZE),
                        L1_KERNEL_DEVICE_FLAGS);
    }

    uint32_t flags = 0x00050C0E;

    /* MAP PHYS MEM */
    for (i = 0; i < MEM_PHYS_SIZE / MEM_SECTION_SIZE; i++)
    {
        MMU_map_section(l1_tables,
                        MEM_PHYS_BASE + (i * MEM_SECTION_SIZE),
                        MEM_PHYS_BASE + (i * MEM_SECTION_SIZE),
                        flags);
    }

    uint32_t index = 0x9fe00000 >> 20; // = 0x9FE

    uart_printf("L1[0x%x] = 0x%x\n", index, l1_tables[index]);

    if (l1_tables[index] & (1 << 4))
    {
        uart_puts("XN IS SET — cannot execute\n");
    }
}

void MMU_init(void)
{
    uint32_t* l1_tables = (uint32_t*) MEM_BOOT_PAGE_TABLE_BASE;
    clear_boot_tables();
    MMU_set_domains();

    /* Later we will map hardware pages 1:1 (and in user memory map) */
    /* We can also enable caching on memory */
    mmu_map_hardware_pages();
    log_message(LOG_LEVEL_INFO, "Mapped all hardware section entries\n");

    set_ttbr0(l1_tables);
    log_message(LOG_LEVEL_INFO,
                "Loaded L1 tables located at 0x%x into TTBR0\n",
                l1_tables);

    flush_tlb();
}

void MMU_enable(void)
{
    flush_tlb();
    flush_i_cache();
    d_cache_enable();
    i_cache_enable();
    _mmu_enable();
    log_message(LOG_LEVEL_INFO, "MMU Enabled!\n");
}

void MMU_disable(void)
{
    _mmu_disable();
    d_cache_disable();
    i_cache_disable();
}

uint32_t read_dacr(void)
{
    uint32_t dacr;
    __asm__ volatile("mrc p15, 0, %0, c3, c0, 0" : "=r"(dacr));
    return dacr;
}

void set_ttbr0(uint32_t* l1_base)
{
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0 \n" // TTBR0
                     "dsb \n"
                     "isb \n"
                     :
                     : "r"(l1_base));
}

uint32_t read_ttbr0(void)
{
    uint32_t ttbr0;
    __asm__ volatile("mrc p15, 0, %0, c2, c0, 0\n" : "=r"(ttbr0));
    return ttbr0;
}

void log_vaddr_mappings(uint32_t* vaddr)
{
    uint32_t* l1_tables = (uint32_t*) MEM_BOOT_PAGE_TABLE_BASE;
    uart_printf("VADDR: 0x%x\n", vaddr);
    log_l1_pte(l1_tables[GET_L1_INDEX((uint32_t) vaddr)]);
}

/* debug log l1 entry */
void log_l1_pte(uint32_t value)
{
    uint32_t section_type =
        value & 0x3; /* Bits 1:0: Should be `0b10` for a section */
    uint32_t b            = (value >> 2) & 1;    /* Bit 2: Bufferable */
    uint32_t c            = (value >> 3) & 1;    /* Bit 3: Cacheable */
    uint32_t xn           = (value >> 4) & 1;    /* Bit 4: eXecute Never (XN) */
    uint32_t domain       = (value >> 5) & 0xF;  /* Bits 8:5: Domain */
    uint32_t ap           = (value >> 10) & 0x3; /* Bits 11:10: AP */
    uint32_t tex          = (value >> 12) & 0x7; /* Bits 14:12: TEX */
    uint32_t ap2          = (value >> 15) & 0x1; /* Bits 15: AP2 */
    uint32_t s            = (value >> 16) & 1;   /* Bit 16: Shareable */
    uint32_t n_g          = (value >> 17) & 1;   /* Bit 17: Not Global */
    uint32_t supersection = (value >> 18) & 1;   /* Bit 18: Supersection flag */
    uint32_t ns           = (value >> 19) & 1;   /* Bit 19: Non-Secure */
    uint32_t section_base = value & 0xFFF00000;  /* Bits 31:20 */

    const char* type_str;
    switch (section_type)
    {
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

    if (supersection != 0)
    {
        uart_printf("Error: Supersection not supported on hardware, bit should "
                    "not be set\n");
        return;
    }

    uart_printf(
        "L1PageTableEntry { base: 0x%x, B: %u, C: %u, AP: %u %u, TEX: %u, "
        "Domain: %u, nG: %u, S: %u, XN: %u, NS: %u, type: %s }\n",
        section_base,
        b,
        c,
        ap2,
        ap,
        tex,
        domain,
        n_g,
        s,
        xn,
        ns,
        type_str);
}

void mmu_copy_bootloader_entries(uint32_t* l1_base)
{
    uint32_t* boot_l1_base = (uint32_t*) MEM_BOOT_PAGE_TABLE_BASE;
    uint32_t i;
    for (i = 0; i < 4096; i++)
    {
        l1_base[i] = boot_l1_base[i];
    }
}

typedef struct frame
{
    uint32_t addr;
    struct frame* next;
} frame_t;
static frame_t frames[MEM_PHYS_SIZE / MEM_SECTION_SIZE];
static frame_t* frame_list = NULL;

/* Allocate frames */
void init_frame_allocator(bootloader_header_t* header)
{
    uint32_t i;
    for (i = header->mapped_sections; i < MEM_PHYS_SIZE / MEM_SECTION_SIZE; i++)
    {
        frame_t* frame = &frames[i];
        uint32_t paddr = MEM_PHYS_BASE + (i * MEM_SECTION_SIZE);
        if (paddr == MEM_BOOT_PAGE_TABLE_BASE)
        { // preserve bootloader page tables
            continue;
        }

        frame->addr = paddr;
        frame->next = frame_list;
        frame_list  = frame;
    }
}

uint32_t alloc_frame(void)
{
    frame_t* frame = frame_list;
    if (frame)
    {
        frame_list = frame->next;
        return frame->addr;
    }
    return 0;
}

void free_frame(uint32_t addr)
{
    /* Check alignment */
    if (addr % MEM_SECTION_SIZE != 0)
    {
        uart_puts("Error: Attempt to free unaligned frame.\n");
        return;
    }

    /* Check address range */
    if (addr < MEM_PHYS_BASE || addr >= (MEM_PHYS_BASE + MEM_PHYS_SIZE))
    {
        uart_puts("Error: Attempt to free address outside valid range.\n");
        return;
    }

    /* Check for double free */
    frame_t* current = frame_list;
    while (current)
    {
        if (current->addr == addr)
        {
            uart_puts("Error: Attempt to free an already freed frame.\n");
            return;
        }
        current = current->next;
    }

    /* Calculate frame index and get the frame pointer */
    uint32_t frame_index = (addr - MEM_PHYS_BASE) / MEM_SECTION_SIZE;
    frame_t* frame       = &frames[frame_index];

    /* Add the frame back to the free list */
    frame->addr = addr;
    frame->next = frame_list;
    frame_list  = frame;

    uart_puts("Frame successfully freed.\n");
}
