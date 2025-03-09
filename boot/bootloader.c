#include <gpio.h>
#include <uart.h>
#include <stdint.h>
#include <interrupt.h>
#include <timer.h>

extern void setup_vbar();
#define LED_PINS    (0xF << 21)
#define LED0        (0x1 << 21)
#define LED1        (0x2 << 21)
#define LED2        (0x4 << 21)
#define LED3        (0x6 << 21)

bool tick_led_on = false;

uint32_t tick_secs = 0;
static inline void delay(unsigned int secs)
{
    uint32_t wait = tick_secs + secs;
    while (tick_secs < wait)
        ;
}

void timer_tick()
{
    if (tick_led_on) GPIO_clear(GPIO1_BASE, LED0);
    else GPIO_set(GPIO1_BASE, LED0);
    tick_led_on = !tick_led_on;
    tick_secs++;
    uart_printf("Tick: %u\n", tick_secs);
}

static inline void gpio_test()
{
    char uart_buffer[100];
    int read = 0;
    unsigned int gpio_base = GPIO1_BASE;
    uint32_t timer_val = 0;

    GPIO_init(); // Currently only configures GPIO1
    GpioSetPinMode(GPIO1_BASE, 0xf << 21, GpioPinOut);
    GPIO_set(GPIO1_BASE, 1 << 21);

    system_interrupt_init();

    // 8N1
    uart_init(0,      // UART index (0 = UART0, 1 = UART1, etc.)
              115200, // Baud rate for communication
              1,      // Stop bit enable (1 = enabled, 0 = disabled)
              0,      // Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)
              0,      // Parity enable (1 = enabled, 0 = disabled)
              0,      // Parity type (0 = even, 1 = odd; ignored if parity is disabled)
              8       // Character length
    );

    timer_init(TIMER2, 1000, timer_tick);
    timer_val = timer_value(TIMER2);
    uart_printf("Timer init value: %u\n", timer_val);
    timer_start(TIMER2);
    timer_val = timer_value(TIMER2);
    uart_printf("Timer counting?. value: %u\n", timer_val);

    while (1)
    {
        delay(5);
        /*
        timer_val = timer_value(TIMER2);
        uart_printf("Timer value: %u\n", timer_val);
        */
        read = uart0_readline(uart_buffer, 100);
        if(read > 0)
        {
            uart_printf("Received: %s", uart_buffer);
        }
    }
}

int min(int a, int b, int c)
{
    if (a <= b && a <= c)
        return a;
    if (b <= a && b <= c)
        return b;
    return c;
}

// Recursive function to compute Levenshtein distance
int levenshtein(const char* str1, const char* str2, int len1, int len2)
{
    // If one of the strings is empty, the distance is the length of the other string
    if (len1 == 0)
        return len2;
    if (len2 == 0)
        return len1;

    // If the characters are the same, no operation is needed
    if (str1[len1 - 1] == str2[len2 - 1])
    {
        return levenshtein(str1, str2, len1 - 1, len2 - 1);
    }

    // Otherwise, consider all possibilities and take the minimum
    volatile int insert  = levenshtein(str1, str2, len1, len2 - 1);     // Insertion
    volatile int remove  = levenshtein(str1, str2, len1 - 1, len2);     // Deletion
    volatile int replace = levenshtein(str1, str2, len1 - 1, len2 - 1); // Substitution

    return 1 + min(insert, remove, replace);
}

extern uintptr_t __BBB_DRAM_BEGIN;

void __BootloaderEntry()
{
    const char* str1  = "kien";
    const char* str2  = "sittineiwog";
    volatile int len1 = 4;
    volatile int len2 = 11;

    if (levenshtein("kien", "sittineiwog", 4, 11) == 9)
    {
        len2 = 8;
        len1 = 20;
        gpio_test(); /* setup_vbar();  // Set the interrupt vector table */
    }

    // mem copy the kernel to dram

    // jump to kernel
    ((void (*)()) __BBB_DRAM_BEGIN)();
}
