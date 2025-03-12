#ifndef UTILS_H
#define UTILS_H

// AOF(array offset), get reg in array form
#define ArrOff(x) (x / sizeof(RegIO))

/**
 * Write a value to a register
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param value  Value to write to the register
 */
void REG32_write(unsigned int base, unsigned int offset, unsigned int value);

/**
 * Write a value to a register with a mask (only writes to the bits specified by the mask)
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param mask   Mask to apply to the value
 * @param value  Value to write to the register (masked bits only)
 */
void REG32_write_masked(unsigned int base, unsigned int offset, unsigned int mask, unsigned int value);
/**
 * Read a value from a register
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @return       Value read from the register
 */
unsigned int REG32_read(unsigned int base, unsigned int offset);

/**
 * Read a value from a register with a mask
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param mask   Mask to apply to the value
 * @return       Value read from the register (masked bits only)
 */
unsigned int REG32_read_masked(unsigned int base, unsigned int offset, unsigned int mask);

// either pass a value in for result or panic, maybe making 2 macros,
// WAIT_FOR_REG32 and WAIT_FOR_REG32_PANIC
#define WAIT_FOR_REG32(base, offset, mask, expected, timeout)  \
    do {                                                      \
        uint32_t _timeout = (timeout);                        \
        while ((REG32_read_masked((base), (offset), (mask)) != (expected)) && (_timeout-- > 0)) { \
        }                                                     \
    } while (0)

#endif /*UTILS_H*/
