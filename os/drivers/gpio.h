#ifndef GPIO_H
#define GPIO_H

/* GPIO Base addresses */
enum GpioIOBase
{
    GPIO0_BASE = 0x44e07000,
    GPIO1_BASE = 0x4804c000,
    GPIO2_BASE = 0x481ac000
};

#define LED_PINS (0xF << 21)
#define LED0     (0x1 << 21)
#define LED1     (0x2 << 21)
#define LED2     (0x4 << 21)
#define LED3     (0x8 << 21)

#define DRIVER_ENA (0x1 << 6)
#define DRIVER_ENB (0x1 << 7)
#define DRIVER_IN1 (0x1 << 2)
#define DRIVER_IN2 (0x1 << 3)
#define DRIVER_IN3 (0x1 << 4)
#define DRIVER_IN4 (0x1 << 5)
#define DRIVER_PINS                                                   \
    (DRIVER_ENA | DRIVER_ENB | DRIVER_IN1 | DRIVER_IN2 | DRIVER_IN3 | \
     DRIVER_IN4)

/* GPIO offsets */
/*----------------------------------------------------
0h GPIO_REVISION Section 25.4.1.1
10h GPIO_SYSCONFIG Section 25.4.1.2
20h GPIO_EOI Section 25.4.1.3
24h GPIO_IRQSTATUS_RAW_0 Section 25.4.1.4
28h GPIO_IRQSTATUS_RAW_1 Section 25.4.1.5
2Ch GPIO_IRQSTATUS_0 Section 25.4.1.6
30h GPIO_IRQSTATUS_1 Section 25.4.1.7
34h GPIO_IRQSTATUS_SET_0 Section 25.4.1.8
38h GPIO_IRQSTATUS_SET_1 Section 25.4.1.9
3Ch GPIO_IRQSTATUS_CLR_0 Section 25.4.1.10
40h GPIO_IRQSTATUS_CLR_1 Section 25.4.1.11
44h GPIO_IRQWAKEN_0 Section 25.4.1.12
48h GPIO_IRQWAKEN_1 Section 25.4.1.13
114h GPIO_SYSSTATUS Section 25.4.1.14
130h GPIO_CTRL Section 25.4.1.15
134h GPIO_OE Section 25.4.1.16
138h GPIO_DATAIN Section 25.4.1.17
13Ch GPIO_DATAOUT Section 25.4.1.18
140h GPIO_LEVELDETECT0 Section 25.4.1.19
144h GPIO_LEVELDETECT1 Section 25.4.1.20
148h GPIO_RISINGDETECT Section 25.4.1.21
14Ch GPIO_FALLINGDETECT Section 25.4.1.22
150h GPIO_DEBOUNCENABLE Section 25.4.1.23
154h GPIO_DEBOUNCINGTIME Section 25.4.1.24
190h GPIO_CLEARDATAOUT Section 25.4.1.25
194h GPIO_SETDATAOUT Section 25.4.1.26
------------------------------------------------------*/

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

void dumb_delay(void);
void GPIO_init(void); /*  Only configures GPIO1 for now */
void GPIO_set(unsigned int gpio_base, unsigned int pins);
void GPIO_clear(unsigned int gpio_base, unsigned int pins);
void GpioSetPinMode(const enum GpioIOBase Gpio,
                    const unsigned int PinMask,
                    const enum GpioPinDirection Dir);
#endif
