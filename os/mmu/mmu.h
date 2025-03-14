// memory API

#include "uart.h"

// #ifdef PLATFORM_BBB
#define PADDR(x) (x)
#define VADDR(x) (x)
// #else
// #define PADDR(x) (x - 0x40000000)
// #define VADDR(x) (x + 0x40000000)
// #endif


/* place the bootloader page tables far off in memory so it doesn't conflict with kernel/clearing bss */
// #ifdef PLATFORM_BBB
// #define BOOTLOADER_PAGE_TABLE_BASE 0x90000000U
// #else
// #define BOOTLOADER_PAGE_TABLE_BASE 0x50000000U
// #endif


#define VIRT_MEM_START 0x80000000
#define VIRT_DRAM_START 0x80000000
#define VIRT_DRAM_END 0x9FFFFFFF

#define SECTION_ADDR_MASK 0xFFF00000

#define L1_SECTION_DESCRIPTOR 0x2
#define L1_PAGE_DESCRIPTOR 0x1

/* Raw permission bits, needs to be shifted into place */
#define RAW_AP_NO_NO 0x0
#define RAW_AP_RW_NO 0x1
#define RAW_AP_RW_RO 0x2
#define RAW_AP_RW_RW 0x3

#define RAW_AP2_0 0
#define RAW_AP2_1 1

/* Raw tex bits, needs to be shifted into place */
#define RAW_TEX_XN 0x0
#define RAW_TEX_XR 0x1
#define RAW_TEX_XRW 0x2
#define RAW_TEX_XRWB 0x3

/* L1 page table AP bit shifts */
#define L1_AP_SHIFT 10
#define L1_AP2_SHIFT 15

/* L2 page table AP bit shifts (Unused) */
#define L2_AP_SHIFT 4
#define L2_AP2_SHIFT 9

/* Other various page option bits */
#define L1_SHAREABLE (1 << 16)
#define L1_CACHEABLE (1 << 3)
#define L1_NOT_GLOBAL (1 << 17)
#define L1_GLOBAL (0 << 17)      /* User pages should be GLOBAL and have an ASID attached */
#define L1_NON_SECURE (1 << 19)

#define L1_ACCESS_NX (1 << 4)
#define L1_ACCESS_X (0 << 4)

/// L1 AP bits for read/write access for KERN_USR
#define L1_ACCESS_NO_NO ((RAW_AP_NO_NO << L1_AP_SHIFT) | (RAW_AP2_0 << L1_AP2_SHIFT))
#define L1_ACCESS_RW_NO ((RAW_AP_RW_NO << L1_AP_SHIFT) | (RAW_AP2_0 << L1_AP2_SHIFT))
#define L1_ACCESS_RW_RO ((RAW_AP_RW_RO << L1_AP_SHIFT) | (RAW_AP2_0 << L1_AP2_SHIFT))
#define L1_ACCESS_RW_RW ((RAW_AP_RW_RW << L1_AP_SHIFT) | (RAW_AP2_0 << L1_AP2_SHIFT))
#define L1_ACCESS_RO_NO ((RAW_AP_RW_NO << L1_AP_SHIFT) | (RAW_AP2_1 << L1_AP2_SHIFT))
#define L1_ACCESS_RO_RO ((RAW_AP_RW_RW << L1_AP_SHIFT) | (RAW_AP2_1 << L1_AP2_SHIFT))

#define L1_KERNEL_CODE_FLAGS \
    (L1_ACCESS_RO_NO | L1_ACCESS_X | L1_SHAREABLE | L1_CACHEABLE | L1_GLOBAL)

#define L1_KERNEL_DATA_FLAGS \
    (L1_ACCESS_RW_NO | L1_ACCESS_NX | L1_SHAREABLE | L1_CACHEABLE | L1_GLOBAL)

/* Data page but with code execution. We shouldn't use this later */
#define L1_KERNEL_DATA_EXEC_FLAGS \
    (L1_ACCESS_RW_NO | L1_SHAREABLE | L1_CACHEABLE | L1_GLOBAL)


/* Main activation */
void mmu_init(void);

/* Enable the MMU, ensure that the MMU has been initialized and TTBR0 is valid. */
void mmu_enable(void);

/* Be VERY careful when calling this. Make sure the PC (or else prefetch abort)
and all other accessable registers don't contain an address
that will be accessed later (or else data fault) */
void mmu_disable(void);

void set_ttbr0(uint32_t *l1_base);
uint32_t read_ttbr0(void);
void mmu_set_domains(void);
void mmu_enable(void);
uint32_t read_dacr(void);

void i_cache_enable(void);
void i_cache_disable(void);
void d_cache_enable(void);
void d_cache_disable(void);

void flush_tlb(void);
void flush_tlb_entry(uint32_t vaddr);
void flush_tlb_asid(uint32_t asid);
void flush_d_cache(void);
void flush_i_cache(void);
