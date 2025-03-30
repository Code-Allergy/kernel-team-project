/* Entry point */
#include <types.h>
#include <boot.h>
#include <uart.h>
#include <mmu.h>
#include <utils.h>
#include <scheduler.h>


/* Pointer to the generated header from the linker */
extern kernel_header_t kernel_header;

__attribute__((section(".text.kmain")))
void kmain(bootloader_header_t* boot_header) {
    int i;
    uart_printf("Hello from kernel! We used %d sections for kernel\n", boot_header->mapped_sections);
    init_frame_allocator(boot_header);
    uart_printf("Frame allocator initialized\n");
    

    scheduler_init();
    boot_test();


    /* setup initial kernel crap, stacks,
    other drivers init, processes, then schedule */

    while(1){
	uart_puts("infinite loop\n");
	for(i = 0; i < 19999; i++);
    }


    panic("Reached end of kernel main!\n");
}
