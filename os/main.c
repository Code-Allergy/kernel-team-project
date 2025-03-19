/* Entry point */
#include <types.h>
#include <boot.h>
#include <uart.h>
#include <mmu.h>
#include <utils.h>

/* pad out bss space for now to view sections */
static unsigned char bss_space[4096];

/* pad out data space for now to view sections */
static unsigned char data_space[4096] = {0x00, 0x01, 0x02, 0x03};

/* Pointer to the generated header from the linker */
extern kernel_header_t kernel_header;

__attribute__((section(".text.kmain"))) void kmain(bootloader_header_t* boot_header) {
    uart_printf("Hello from kernel! We used %d sections for kernel\n", boot_header->mapped_sections);
    init_frame_allocator(boot_header);
    uart_printf("Frame allocator initialized\n");
    uint32_t addr = alloc_frame();
    uart_printf("Allocated frame at 0x%x\n", addr);



    panic("End of kernel main!\n");
}
