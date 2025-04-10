#include <gpio.h>
#include <uart.h>
#include <interrupt.h>
#include <ddr.h>
#include <timer.h>
#include <mmc.h>
#include <mmu.h>
#include <mem.h>
#include <boot.h>
#include <utils.h>
#include <fat32.h>
#include <syscall.h>
#include <motor.h>

#define KERNEL_MAGIC     0x1B1B1B1B
#define BOOTLOADER_MAGIC 0x2B2B2B2B

extern void setup_vbar(void);
#define LED_PINS (0xF << 21)
#define LED0     (0x1 << 21)
#define LED1     (0x2 << 21)
#define LED2     (0x4 << 21)
#define LED3     (0x6 << 21)

bool tick_led_on = false;

uint32_t tick_secs = 0;

void delay(unsigned int secs)
{
    uint32_t wait = tick_secs + secs;
    while (tick_secs < wait)
        ;
}
void timer_tick(void)
{

    if (tick_led_on)
        GPIO_clear(GPIO1_BASE, LED0);
    else
        GPIO_set(GPIO1_BASE, LED0);
    tick_led_on = !tick_led_on;
    tick_secs++;
    uart_printf("Tick: %u\n", tick_secs);
}

static inline void gpio_test(void)
{
    char uart_buffer[100];
    int read = 0;
    int a, b;
    uint32_t timer_val = 0;

    GPIO_init(); /* Currently only configures GPIO1*/
    GpioSetPinMode(GPIO1_BASE, 0xf << 21, GpioPinOut);
    GPIO_set(GPIO1_BASE, 1 << 21);

    system_interrupt_init();

    /* 8N1*/
    uart_init(
        0,      /* UART index (0 = UART0, 1 = UART1, etc.)*/
        115200, /* Baud rate for communication*/
        1,      /* Stop bit enable (1 = enabled, 0 = disabled)*/
        0,      /* Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)*/
        0,      /* Parity enable (1 = enabled, 0 = disabled)*/
        0, /* Parity type (0 = even, 1 = odd; ignored if parity is disabled)*/
        8  /* Character length*/
    );

    /*
    timer_init(TIMER2, 1000, timer_tick);
    timer_val = timer_value(TIMER2);
    uart_printf("Timer init value: %u\n", timer_val);
    timer_start(TIMER2);
    timer_val = timer_value(TIMER2);
    uart_printf("Timer counting?. value: %u\n", timer_val);
    */

    /*
    while (1)
    {
        delay(1);

        timer_val = timer_value(TIMER2);
        uart_printf("Timer value: %u\n", timer_val);

        read = uart_readline(1, uart_buffer, 100);
        if(read == 5)
        {
            a = (int)uart_buffer[1] - 128;
            b = (int)uart_buffer[3] - 128;
            uart_printf("Received: x:%d, y:%d\n", a, b);
        }
    }
    */
}

int min(int a, int b, int c)
{
    if (a <= b && a <= c)
        return a;
    if (b <= a && b <= c)
        return b;
    return c;
}

/* Recursive function to compute Levenshtein distance*/
int levenshtein(const char* str1, const char* str2, int len1, int len2)
{
    /* If one of the strings is empty, the distance is the length of the other
     * string*/
    if (len1 == 0)
        return len2;
    if (len2 == 0)
        return len1;

    /* If the characters are the same, no operation is needed*/
    if (str1[len1 - 1] == str2[len2 - 1])
    {
        return levenshtein(str1, str2, len1 - 1, len2 - 1);
    }

    /* Otherwise, consider all possibilities and take the minimum*/
    volatile int insert =
        levenshtein(str1, str2, len1, len2 - 1); /* Insertion*/
    volatile int remove = levenshtein(str1, str2, len1 - 1, len2); /* Deletion*/
    volatile int replace =
        levenshtein(str1, str2, len1 - 1, len2 - 1); /* Substitution*/

    return 1 + min(insert, remove, replace);
}

fat32_fs_t fs;
fat32_diskio_t diskio;
fat32_file_t file;
bootloader_header_t boot_header;
kernel_header_t* kernel_header;

void __BootloaderEntry(void)
{
    int32_t i, res;
    int32_t current_frame;
    int32_t text_section_size, data_section_size, bss_section_size;
    int32_t kernel_sections, kernel_size;
    int32_t bss_section_offset, bss_section_paddr;

    GPIO_init(); /* Currently only configures GPIO1*/
    GpioSetPinMode(GPIO1_BASE, 0xf << 21, GpioPinOut);
    GPIO_set(GPIO1_BASE, 1 << 21);

    system_interrupt_init();

    /* 8N1*/
    uart_init(
        0,      /* UART index (0 = UART0, 1 = UART1, etc.)*/
        115200, /* Baud rate for communication*/
        1,      /* Stop bit enable (1 = enabled, 0 = disabled)*/
        0,      /* Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)*/
        0,      /* Parity enable (1 = enabled, 0 = disabled)*/
        0, /* Parity type (0 = even, 1 = odd; ignored if parity is disabled)*/
        8  /* Character length*/
    );
    log_message(LOG_LEVEL_INFO, "Bootloader started\n");

    log_message(LOG_LEVEL_INFO, "Init DRAM\n");
    setup_memory(); /*  Initialize DDR3 */
    log_message(LOG_LEVEL_INFO, "Done DRAM!\n");
    int result = test_ddr3_memory();
    if (result == 0)
    {
        /*  Success - Memory is functioning correctly */
        log_message(LOG_LEVEL_INFO, "DDR Memory test SUCCESS\n");
    }
    else
    {
        /*  Failure - Memory test failed */
        log_message(LOG_LEVEL_WARN, "DDR Memory test FAILED\n");
    }
    log_message(LOG_LEVEL_INFO, "Init ddr end\n");

    log_message(LOG_LEVEL_INFO, "MMC Init\n");
    mmc_controller_init();

    log_message(LOG_LEVEL_INFO, "FAT32 Init\n");
    /* Copy entire kernel image into memory at MEM_PHYS_BASE */
    diskio.read_sector = &mmc_read_sector;
    if ((res = fat32_mount(&fs, &diskio)) != 0)
    {
        panic("Failed to mount FAT32 filesystem: %s\n", fat32_geterror(res));
    }
    log_message(LOG_LEVEL_INFO, "Mounted FAT32 filesystem\n");
    if ((res = fat32_open(&fs, "/boot/kernel.bin", &file)) != 0)
    {
        panic("Failed to open kernel.bin: %s\n", fat32_geterror(res));
    };
    log_message(LOG_LEVEL_INFO, "Opened kernel.bin\n");

    /* Copy the kernel directly into memory */
    kernel_size = fat32_size(&file);
    if ((res = fat32_read(&file, (void*) MEM_PHYS_BASE, kernel_size)) !=
        kernel_size)
    {
        panic("Failed to read kernel.bin: %s (%d bytes)\n",
              fat32_geterror(res),
              res);
    }
    log_message(
        LOG_LEVEL_INFO, "Copied kernel into memory at 0x%x\n", MEM_PHYS_BASE);
    kernel_header = (kernel_header_t*) MEM_PHYS_BASE;

    /* Verify the kernel has the expected header */
    if (kernel_header->magic != KERNEL_MAGIC)
    {
        panic("Invalid kernel magic value: %x\n", kernel_header->magic);
    }

    /* Set some values from the header */
    text_section_size = kernel_header->data_start - kernel_header->text_start;
    data_section_size = kernel_header->bss_start - kernel_header->data_start;
    bss_section_size  = kernel_header->kernel_size - data_section_size;
    kernel_sections =
        (kernel_header->kernel_size + MEM_SECTION_SIZE - 1) / MEM_SECTION_SIZE;
    bss_section_offset = kernel_header->bss_start - MEM_KERNEL_BASE;
    bss_section_paddr  = (MEM_PHYS_BASE + bss_section_offset);

    log_message(LOG_LEVEL_INFO, "MMU Init\n");
    MMU_init();

    /* Create kernel code page mappings, RWX pages for simplicity */
    for (current_frame = 0; current_frame < kernel_sections; current_frame++)
    {
        MMU_map_section(MEM_BOOT_PAGE_TABLE_BASE,
                        MEM_KERNEL_BASE + (current_frame * MEM_SECTION_SIZE),
                        MEM_PHYS_BASE + (current_frame * MEM_SECTION_SIZE),
                        L1_KERNEL_DATA_EXEC_FLAGS);
        log_vaddr_mappings((uint32_t*) (kernel_header->text_start +
                                        (current_frame * MEM_SECTION_SIZE)));
    }
    log_message(LOG_LEVEL_INFO, "Done mapping kernel code/data page!\n");

    log_message(LOG_LEVEL_INFO, "MMU Enable\n");
    MMU_enable();

    /* clear bss section */
    for (i = 0; i < bss_section_size; i += 4)
    {
        *((uint32_t*) (bss_section_paddr + i)) = 0;
    }

    /* Setup whatever info is needed from bootloader */
    boot_header.magic                 = BOOTLOADER_MAGIC;
    boot_header.mapped_sections       = kernel_sections;
    boot_header.boot_table_entry_addr = MEM_BOOT_PAGE_TABLE_BASE;

    /* for now, copy the instruction to jump to the kernel entry to the kernel
     * entry point */
    log_message(LOG_LEVEL_INFO,
                "Jumping to kernel entry at %x\n",
                kernel_header->kernel_entry);
    ((void (*)(bootloader_header_t*)) kernel_header->kernel_entry)(
        &boot_header);

    panic("Reached end of bootloader main without jumping!\n");
}
