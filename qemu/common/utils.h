#include <uart.h>

#ifndef UTILS_H
    #define UTILS_H

    /* AOF(array offset), get reg in array form */
    #define ArrOff(x) (x / sizeof(RegIO))

void check_mode(void);

/**
 * Write a value to a register
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param value  Value to write to the register
 */
static inline void
    REG32_write(unsigned int base, unsigned int offset, unsigned int value)
{
    volatile unsigned int* reg = (volatile unsigned int*) (base + offset);
    *reg                       = value;
}

/**
 * Write a value to a register with a mask (only writes to the bits specified by
 * the mask)
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param mask   Mask to apply to the value
 * @param value  Value to write to the register (masked bits only)
 */
static inline void REG32_write_masked(unsigned int base,
                                      unsigned int offset,
                                      unsigned int mask,
                                      unsigned int value)
{
    volatile unsigned int* reg = (volatile unsigned int*) (base + offset);
    *reg                       = (*reg & ~mask) | (value & mask);
}

/**
 * Read a value from a register
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @return       Value read from the register
 */
static inline unsigned int REG32_read(unsigned int base, unsigned int offset)
{
    volatile unsigned int* reg = (volatile unsigned int*) (base + offset);
    return *reg;
}

/**
 * Read a value from a register with a mask
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param mask   Mask to apply to the value
 * @return       Value read from the register (masked bits only)
 */
static inline unsigned int
    REG32_read_masked(unsigned int base, unsigned int offset, unsigned int mask)
{
    volatile unsigned int* reg = (volatile unsigned int*) (base + offset);
    return *reg & mask;
}

    /* either pass a value in for result or panic, maybe making 2 macros, */
    /* WAIT_FOR_REG32 and WAIT_FOR_REG32_PANIC */
    #define WAIT_FOR_REG32(base, offset, mask, expected, timeout)              \
        do                                                                     \
        {                                                                      \
            uint32_t _timeout = (timeout);                                     \
            while (                                                            \
                (REG32_read_masked((base), (offset), (mask)) != (expected)) && \
                (_timeout-- > 0))                                              \
            {                                                                  \
            }                                                                  \
        } while (0)

    #define LOG_LEVEL_DEBUG 3
    #define LOG_LEVEL_INFO  2
    #define LOG_LEVEL_WARN  1
    #define LOG_LEVEL_ERROR 0

    #ifndef LOG_LEVEL
        #define LOG_LEVEL LOG_LEVEL_INFO /*  Default log level if not defined \
                                          */
    #endif

static inline void log_message(int level, const char* fmt, ...)
{
    if (level <= LOG_LEVEL)
    { /*  Only log if within the allowed threshold */
        va_list ap;
        va_start(ap, fmt);
        uart_vprintf(fmt, ap);
        va_end(ap);
    }
}

static inline void panic(const char* fmt, ...)
{
    va_list ap;
    uart_printf("PANIC: ");
    va_start(ap, fmt);
    uart_vprintf(fmt, ap);
    va_end(ap);
    /* disable interrupts */
    __asm__ volatile("cpsid i");

    /* can log out some registers or critical info */
    while (1)
    {
        __asm__ volatile("wfi");
    }
}

#endif /*UTILS_H*/
