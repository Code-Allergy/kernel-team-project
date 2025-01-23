#include <gpio.h>

void GPIO_init(void) {
    // Pointer to the base address of GPIO1
    volatile unsigned int *gpio_base = (volatile unsigned int *)GPIO1_BASE;
    volatile unsigned int *gpio_sysconfig = (volatile unsigned int *)(GPIO1_BASE + GPIO_SYSCONFIG);
    volatile unsigned int *gpio_sysstatus = (volatile unsigned int *)(GPIO1_BASE + GPIO_SYSSTATUS);
    volatile unsigned int *gpio_control = (volatile unsigned int *)(GPIO1_BASE + GPIO_CONTROL);
    volatile unsigned int *gpio_nOE = (volatile unsigned int *)(GPIO1_BASE + GPIO_nOE);
   
    unsigned int cm_per_base = 0x44e00000;
    volatile unsigned int *cm_per_clk_l4ls = (volatile unsigned int *)(cm_per_base + CLK_L4LS);
    volatile unsigned int *cm_per_mod_gpio1 = (volatile unsigned int *)(cm_per_base + MOD_GPIO1);
    *cm_per_clk_l4ls = 2;
    *cm_per_mod_gpio1 = 2 | (1 << 18);

    while ((*cm_per_mod_gpio1 & (3 << 16)));

    // Step 1: Write 2 to GPIO_SYSCONFIG
    *gpio_sysconfig = 2;

    // Step 2: Wait for GPIO_SYSSTATUS bit 0 to be set
    while (!(*gpio_sysstatus & 1));
    //while (!(*(gpio_base + (GPIO_SYSSTATUS / 4)) & 1));

    // Step 3: Write 0x10 to GPIO_SYSCONFIG
    *gpio_sysconfig = 0x10;

    // Step 4: Write 0 to GPIO_CONTROL
    *gpio_control = 0;

    // Step 5: Configure Output Enable (OE) register
    unsigned int value = ~(0xF << 21);  // Calculate the value to set in GPIO_nOE
    *gpio_nOE = value;
}


void GPIO_set(unsigned int gpio_base, unsigned int pins) {
    volatile unsigned int* gpio_data_r = (unsigned int *)(gpio_base + GPIO_SET);
    *gpio_data_r = pins;
}

void GPIO_clear(unsigned int gpio_base, unsigned int pins) {
    volatile unsigned int* gpio_data_r = (unsigned int *)(gpio_base + GPIO_CLR);
    *gpio_data_r = pins;
}