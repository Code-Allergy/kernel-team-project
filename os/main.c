/* Entry point */
#include <types.h>
#include <boot.h>
#include <uart.h>
#include <mmu.h>
#include <utils.h>
#include <scheduler.h>
#include <motor.h>
#include <interrupt.h>
#include <gpio.h>

/* Pointer to the generated header from the linker */
extern kernel_header_t kernel_header;

__attribute__((section(".text.kmain"))) void
    kmain(bootloader_header_t* boot_header)
{
    int i;

    check_mode();

    uart_printf("Hello from kernel! We used %d sections for kernel\n",
                boot_header->mapped_sections);
    init_frame_allocator(boot_header);
    uart_printf("Frame allocator initialized\n");

    /* reinitialize interrupts with kernel controlled tables */
    system_interrupt_init();
    uart_init(
        0,      /* UART index (0 = UART0, 1 = UART1, etc.)*/
        115200, /* Baud rate for communication*/
        1,      /* Stop bit enable (1 = enabled, 0 = disabled)*/
        0,      /* Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)*/
        0,      /* Parity enable (1 = enabled, 0 = disabled)*/
        0, /* Parity type (0 = even, 1 = odd; ignored if parity is disabled)*/
        8  /* Character length*/
    );
    log_message(LOG_LEVEL_INFO, "kernel interrupts OK\n");

    uart_printf("UART1 init\n");
    uart_init(
        1,      /* UART index (0 = UART0, 1 = UART1, etc.)*/
        115200, /* Baud rate for communication*/
        1,      /* Stop bit enable (1 = enabled, 0 = disabled)*/
        0,      /* Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)*/
        0,      /* Parity enable (1 = enabled, 0 = disabled)*/
        0, /* Parity type (0 = even, 1 = odd; ignored if parity is disabled)*/
        8  /* Character length*/
    );

    // GPIO_clear(GPIO1_BASE, LED_PINS);
    //  motor_test_sequence(); /* infinite loop */

    check_mode();
    scheduler_init(); /*Creates MVP processes */
    check_mode();
    scheduler_run();
    panic("Reached end of kernel main!\n");
}
