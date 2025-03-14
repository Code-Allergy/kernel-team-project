/* Entry point */
#include <types.h>


/* pad out bss space for now to view sections */
static unsigned char bss_space[4096];

/* pad out data space for now to view sections */
static unsigned char data_space[4096] = {0x00, 0x01, 0x02, 0x03};

/* temp here, can be in boot.h or general or something. */
/* we don't really need this in kernel, just in bl */
#define KERNEL_MAGIC 0x1B1B1B1B
typedef struct {
    uint32_t magic;
    uint32_t text_start;
    uint32_t data_start;
    uint32_t bss_start;
    uint32_t kernel_size;
    uint32_t kernel_entry;
    uint32_t reserved[10];
} kernel_header_t;
extern kernel_header_t kernel_header;

void kmain(void) {
    while(1);
}
