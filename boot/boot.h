#ifndef BOOT_H
#define BOOT_H
/* idk where we should put this header */
typedef struct {
    uint32_t magic;
    uint32_t text_start;
    uint32_t data_start;
    uint32_t bss_start;
    uint32_t kernel_size;
    uint32_t kernel_entry;
    uint32_t kernel_end;
    uint32_t reserved[9];
} kernel_header_t;


typedef struct bootloader_header {
    /* magic value to verify header */
    uint32_t magic;

    /* whatever we need to pass to kernel */
    uint32_t boot_table_entry_addr;

    /* mapped sections before passing to kernel, unused memory at BASE + (1M*mapped_sections) */
    uint32_t mapped_sections;
} bootloader_header_t;

#endif /* BOOT_H */
