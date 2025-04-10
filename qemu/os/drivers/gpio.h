#ifndef GPIO_H
#define GPIO_H

enum GpioIOBase
{
    GPIO0_BASE = 0x00000000,
    GPIO1_BASE = 0x00000000,
    GPIO2_BASE = 0x00000000
};

enum GpioIORegister
{
    GPIO_REVISION_OFF        = 0x000,
    GPIO_SYSCONFIG_OFF       = 0x010,
    GPIO_EOI_OFF             = 0x020,
    GPIO_IRQSTATUS_RAW_0_OFF = 0x024,
    GPIO_IRQSTATUS_RAW_1_OFF = 0x028,
    GPIO_IRQSTATUS_0_OFF     = 0x02c,
    GPIO_IRQSTATUS_1_OFF     = 0x030,
    GPIO_IRQSTATUS_SET_0_OFF = 0x034,
    GPIO_IRQSTATUS_SET_1_OFF = 0x038,
    GPIO_IRQSTATUS_CLR_0_OFF = 0x03c,
    GPIO_IRQSTATUS_CLR_1_OFF = 0x040,
    GPIO_IRQWAKEN_0_OFF      = 0x044,
    GPIO_IRQWAKEN_1_OFF      = 0x048,
    GPIO_SYSSTATUS_OFF       = 0x114,
    GPIO_CTRL_OFF            = 0x130,
    GPIO_OE_OFF              = 0x134,
    GPIO_DATAIN_OFF          = 0x138,
    GPIO_DATAOUT_OFF         = 0x13c,
    GPIO_LEVELDETECT0_OFF    = 0x140,
    GPIO_LEVELDETECT1_OFF    = 0x144,
    GPIO_RISINGDETECT_OFF    = 0x148,
    GPIO_FALLINGDETECT_OFF   = 0x14c,
    GPIO_DEBOUNCENABLE_OFF   = 0x150,
    GPIO_DEBOUNCINGTIME_OFF  = 0x154,
    GPIO_CLEARDATAOUT_OFF    = 0x190,
    GPIO_SETDATAOUT_OFF      = 0x194,

    CLK_L4LS_OFF  = 0x000,
    MOD_GPIO1_OFF = 0x0ac, /* debounce clock enable in bit 18 */
    MOD_GPIO2_OFF = 0x0b0, /* debounce clock enable in bit 18 */
    MOD_GPIO3_OFF = 0x0b4  /* debounce clock enable in bit 18 */
};

enum GpioPinDirection
{
    GpioPinOut = 0,
    GpioPinIn  = 1
};

void GPIO_init(void); /*  Only configures GPIO1 for now */
void GPIO_set(unsigned int gpio_base, unsigned int pins);
void GPIO_clear(unsigned int gpio_base, unsigned int pins);
void GpioSetPinMode(const enum GpioIOBase Gpio,
                    const unsigned int PinMask,
                    const enum GpioPinDirection Dir);

#endif /* GPIO_H */
