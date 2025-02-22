#include <utils.h>

/**
 * Write a value to a register
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param value  Value to write to the register
 */
void REG32_write(unsigned int base, unsigned int offset, unsigned int value)
{
    volatile unsigned int* reg = (volatile unsigned int*) (base + offset);
    *reg                       = value;
}

/**
 * Write a value to a register with a mask (only writes to the bits specified by the mask)
 *
 * @param base   Base address of the register
 * @param offset Offset of the register
 * @param mask   Mask to apply to the value
 * @param value  Value to write to the register (masked bits only)
 */
void REG32_write_masked(unsigned int base, unsigned int offset, unsigned int mask, unsigned int value)
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
unsigned int REG32_read(unsigned int base, unsigned int offset)
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
unsigned int REG32_read_masked(unsigned int base, unsigned int offset, unsigned int mask)
{
    volatile unsigned int* reg = (volatile unsigned int*) (base + offset);
    return *reg & mask;
}
