/* Entry point */
#include <types.h>
#include <boot.h>

/* pad out bss space for now to view sections */
static unsigned char bss_space[4096];

/* pad out data space for now to view sections */
static unsigned char data_space[4096] = {0x00, 0x01, 0x02, 0x03};

/* Pointer to the generated header from the linker */
extern kernel_header_t kernel_header;

void kmain(bootloader_header_t* boot_header) {
    (void)boot_header;
    while(1);
}
