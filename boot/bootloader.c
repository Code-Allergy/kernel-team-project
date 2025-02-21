#include <gpio.h>
#include <uart.h>

extern void setup_vbar();
#define LED_PINS (0xF << 21)

static inline void delay(volatile unsigned int count)
{
    while (count--)
        ;
}

void dumb_delay()
{
    volatile unsigned int count = 0x3FFFFFF;
    while (count--)
        ;
}

static inline void gpio_test()
{
    unsigned int gpio_base = GPIO1_BASE;

    delay(0xFFFF);
    GPIO_init(); // Currently only configures GPIO1
    GpioSetPinMode(GPIO1_BASE, 0xf << 21, GpioPinOut);
    GPIO_set(GPIO1_BASE, 1 << 21);
    // 8N1
    uart_init(0,      // UART index (0 = UART0, 1 = UART1, etc.)
              115200, // Baud rate for communication
              1,      // Stop bit enable (1 = enabled, 0 = disabled)
              0,      // Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)
              0,      // Parity enable (1 = enabled, 0 = disabled)
              0,      // Parity type (0 = even, 1 = odd; ignored if parity is disabled)
              8       // Character length
    );

    /* uart_puts("Sup bro\n"); */

    while (1)
    {
        GPIO_set(gpio_base, LED_PINS);
        uart_puts("LEDs on!\n");
        delay(0x1FFFFFF);
        GPIO_clear(gpio_base, LED_PINS);
        uart_puts("LEDs off!\n");
        delay(0x1FFFFFF);
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
}
