#include <uart.h>
#include <utils.h>
#include <clock_module.h>
#include <mmc.h>
#include <gpio.h>
#include <hw_cm_wkup.h>

#define LED_PINS (0xF << 21)

/*PLL*/
#define SOC_PRCM_REGS                        (0x44E00000)

#define SOC_CM_WKUP_REGS                     (SOC_PRCM_REGS + 0x400)

/**Setting the CORE PLL values at OPP100:
** OSCIN = 24MHz, Fdpll = 2GHz
** HSDM4 = 200MHz, HSDM5 = 250MHz
** HSDM6 = 500MHz
*/
#define COREPLL_M                          1000
#define COREPLL_N                          23
#define COREPLL_HSD_M4                     10
#define COREPLL_HSD_M5                     8
#define COREPLL_HSD_M6                     4

/* Setting the  PER PLL values at OPP100:
** OSCIN = 24MHz, Fdpll = 960MHz
** CLKLDO = 960MHz, CLKOUT = 192MHz
*/
#define PERPLL_M                           960
#define PERPLL_N                           23
#define PERPLL_M2                          5



/*move this 2*/
#define CONTROL_MODULE_BASE 0x44E10000
#define CLK32KDIVRATIO_CTRL 0x444
/*SD card command timeout error*/
#define CTO_ERROR -16
#define CC 0


/* \brief This function initializes the CORE PLL 
 * 
 * \param none
 *
 * \return none
 *
 */
void CorePLLInit(void)
{
    volatile unsigned int regVal = 0;

    /* Enable the Core PLL */

    /* Put the PLL in bypass mode */
    regVal = REG32_read(SOC_CM_WKUP_REGS,  CM_WKUP_CM_CLKMODE_DPLL_CORE) &
                ~CM_WKUP_CM_CLKMODE_DPLL_CORE_DPLL_EN;

    regVal |= CM_WKUP_CM_CLKMODE_DPLL_CORE_DPLL_EN_DPLL_MN_BYP_MODE;

    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_CORE, regVal);

    while(!(REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_IDLEST_DPLL_CORE) &
                      CM_WKUP_CM_IDLEST_DPLL_CORE_ST_MN_BYPASS));

    /* Set the multipler and divider values for the PLL */
    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKSEL_DPLL_CORE,
        ((COREPLL_M << CM_WKUP_CM_CLKSEL_DPLL_CORE_DPLL_MULT_SHIFT) |
         (COREPLL_N << CM_WKUP_CM_CLKSEL_DPLL_CORE_DPLL_DIV_SHIFT)));

    /* Configure the High speed dividers */
    /* Set M4 divider */    
    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M4_DPLL_CORE);
    regVal = regVal & ~CM_WKUP_CM_DIV_M4_DPLL_CORE_HSDIVIDER_CLKOUT1_DIV;
    regVal = regVal | (COREPLL_HSD_M4 << 
                CM_WKUP_CM_DIV_M4_DPLL_CORE_HSDIVIDER_CLKOUT1_DIV_SHIFT);
    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M4_DPLL_CORE, regVal);
    
    /* Set M5 divider */    
    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M5_DPLL_CORE);
    regVal = regVal & ~CM_WKUP_CM_DIV_M5_DPLL_CORE_HSDIVIDER_CLKOUT2_DIV;
    regVal = regVal | (COREPLL_HSD_M5 << 
                CM_WKUP_CM_DIV_M5_DPLL_CORE_HSDIVIDER_CLKOUT2_DIV_SHIFT);
    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M5_DPLL_CORE, regVal);
        
    /* Set M6 divider */    
    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M6_DPLL_CORE);
    regVal = regVal & ~CM_WKUP_CM_DIV_M6_DPLL_CORE_HSDIVIDER_CLKOUT3_DIV;
    regVal = regVal | (COREPLL_HSD_M6 << 
                CM_WKUP_CM_DIV_M6_DPLL_CORE_HSDIVIDER_CLKOUT3_DIV_SHIFT);
    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M6_DPLL_CORE, regVal);

    /* Now LOCK the PLL by enabling it */
    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_CORE) &
                ~CM_WKUP_CM_CLKMODE_DPLL_CORE_DPLL_EN;

    regVal |= CM_WKUP_CM_CLKMODE_DPLL_CORE_DPLL_EN;

    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_CORE, regVal);

    while(!(REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_IDLEST_DPLL_CORE) &
                        CM_WKUP_CM_IDLEST_DPLL_CORE_ST_DPLL_CLK));
}



/* \brief This function initializes the PER PLL
 * 
 * \param none
 *
 * \return none
 *
 */
void PerPLLInit(void)
{
    volatile unsigned int regVal = 0;
    volatile unsigned int reg2 = 0;

    /* Put the PLL in bypass mode */
    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_PER) &
                ~CM_WKUP_CM_CLKMODE_DPLL_PER_DPLL_EN;

    regVal |= CM_WKUP_CM_CLKMODE_DPLL_PER_DPLL_EN_DPLL_MN_BYP_MODE;

    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_PER, regVal);

    while(!(REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_IDLEST_DPLL_PER) &
                      CM_WKUP_CM_IDLEST_DPLL_PER_ST_MN_BYPASS));

    reg2 = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKSEL_DPLL_PERIPH);
    reg2 &= ~(CM_WKUP_CM_CLKSEL_DPLL_PERIPH_DPLL_MULT | 
            CM_WKUP_CM_CLKSEL_DPLL_PERIPH_DPLL_DIV);
    
    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKSEL_DPLL_PERIPH, reg2);

    reg2 = 0;

    /* Set the multipler and divider values for the PLL */
    reg2 = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKSEL_DPLL_PERIPH);
    reg2 |= ((PERPLL_M << CM_WKUP_CM_CLKSEL_DPLL_PERIPH_DPLL_MULT_SHIFT) |
         (PERPLL_N << CM_WKUP_CM_CLKSEL_DPLL_PERIPH_DPLL_DIV_SHIFT));

    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKSEL_DPLL_PERIPH, reg2);

    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M2_DPLL_PER);
    regVal = regVal & ~CM_WKUP_CM_DIV_M2_DPLL_PER_DPLL_CLKOUT_DIV;
    regVal = regVal | PERPLL_M2;

    /* Set the CLKOUT2 divider */
    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_DIV_M2_DPLL_PER, regVal);
    
    /* Now LOCK the PLL by enabling it */
    regVal = REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_PER) &
                ~CM_WKUP_CM_CLKMODE_DPLL_PER_DPLL_EN;

    regVal |= CM_WKUP_CM_CLKMODE_DPLL_PER_DPLL_EN;

    REG32_write(SOC_CM_WKUP_REGS, CM_WKUP_CM_CLKMODE_DPLL_PER, regVal);

    while(!(REG32_read(SOC_CM_WKUP_REGS, CM_WKUP_CM_IDLEST_DPLL_PER) &
                           CM_WKUP_CM_IDLEST_DPLL_PER_ST_DPLL_CLK));

}

void delay(volatile unsigned int count)
{
    while (count--);
}


/*GPIO for waiting*/
void LED_DELAY(int i)
{
    unsigned int gpio_base = GPIO1_BASE;

    delay(0xFFFF);
    GPIO_init(); // Currently only configures GPIO1
    GPIO_set(GPIO1_BASE, 1 << 21);
    // 8N1
    /* uart_init( */
    /*     0,         // UART index (0 = UART0, 1 = UART1, etc.) */
    /*     115200,    // Baud rate for communication */
    /*     1,         // Stop bit enable (1 = enabled, 0 = disabled) */
    /*     0,         // Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits) */
    /*     0,         // Parity enable (1 = enabled, 0 = disabled) */
    /*     0,         // Parity type (0 = even, 1 = odd; ignored if parity is disabled) */
    /*     8          // Character length */
    /* ); */

    /* uart_puts("Sup bro\n"); */

    while (i > 0)
    {
        GPIO_set(gpio_base, LED_PINS);
        /* uart_puts("LEDs on!\n"); */
        delay(0x1FFFFFF);
        GPIO_clear(gpio_base, LED_PINS);
        /* uart_puts("LEDs off!\n"); */
        delay(0x1FFFFFF);
        i--;
    }
}



/*Pin Muxing*/
void configure_mmc_pins(void) {
    REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_MMC0_DAT3,
                (0 << CONTROL_CONF_MMC0_DAT3_CONF_MMC0_DAT3_MMODE_SHIFT)    |
                (0 << CONTROL_CONF_MMC0_DAT3_CONF_MMC0_DAT3_PUDEN_SHIFT)    |
                (1 << CONTROL_CONF_MMC0_DAT3_CONF_MMC0_DAT3_PUTYPESEL_SHIFT)|
                (1 << CONTROL_CONF_MMC0_DAT3_CONF_MMC0_DAT3_RXACTIVE_SHIFT));

    REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_MMC0_DAT2,
                (0 << CONTROL_CONF_MMC0_DAT2_CONF_MMC0_DAT2_MMODE_SHIFT)    |
                (0 << CONTROL_CONF_MMC0_DAT2_CONF_MMC0_DAT2_PUDEN_SHIFT)    |
                (1 << CONTROL_CONF_MMC0_DAT2_CONF_MMC0_DAT2_PUTYPESEL_SHIFT)|
                (1 << CONTROL_CONF_MMC0_DAT2_CONF_MMC0_DAT2_RXACTIVE_SHIFT));

    REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_MMC0_DAT1,
                (0 << CONTROL_CONF_MMC0_DAT1_CONF_MMC0_DAT1_MMODE_SHIFT)    |
                (0 << CONTROL_CONF_MMC0_DAT1_CONF_MMC0_DAT1_PUDEN_SHIFT)    |
                (1 << CONTROL_CONF_MMC0_DAT1_CONF_MMC0_DAT1_PUTYPESEL_SHIFT)|
                (1 << CONTROL_CONF_MMC0_DAT1_CONF_MMC0_DAT1_RXACTIVE_SHIFT));

    REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_MMC0_DAT0,
                (0 << CONTROL_CONF_MMC0_DAT0_CONF_MMC0_DAT0_MMODE_SHIFT)    |
                (0 << CONTROL_CONF_MMC0_DAT0_CONF_MMC0_DAT0_PUDEN_SHIFT)    |
                (1 << CONTROL_CONF_MMC0_DAT0_CONF_MMC0_DAT0_PUTYPESEL_SHIFT)|
                (1 << CONTROL_CONF_MMC0_DAT0_CONF_MMC0_DAT0_RXACTIVE_SHIFT));

    REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_MMC0_CLK,
                (0 << CONTROL_CONF_MMC0_CLK_CONF_MMC0_CLK_MMODE_SHIFT)    |
                (0 << CONTROL_CONF_MMC0_CLK_CONF_MMC0_CLK_PUDEN_SHIFT)    |
                (1 << CONTROL_CONF_MMC0_CLK_CONF_MMC0_CLK_PUTYPESEL_SHIFT)|
                (1 << CONTROL_CONF_MMC0_CLK_CONF_MMC0_CLK_RXACTIVE_SHIFT));

    REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_MMC0_CMD,
                   (0 << CONTROL_CONF_MMC0_CMD_CONF_MMC0_CMD_MMODE_SHIFT)    |
                   (0 << CONTROL_CONF_MMC0_CMD_CONF_MMC0_CMD_PUDEN_SHIFT)    |
                   (1 << CONTROL_CONF_MMC0_CMD_CONF_MMC0_CMD_PUTYPESEL_SHIFT)|
                   (1 << CONTROL_CONF_MMC0_CMD_CONF_MMC0_CMD_RXACTIVE_SHIFT));

     REG32_write(CONTROL_MODULE_BASE, CONTROL_CONF_SPI0_CS1,
                   (5 << CONTROL_CONF_SPI0_CS1_CONF_SPI0_CS1_MMODE_SHIFT)    |
                   (0 << CONTROL_CONF_SPI0_CS1_CONF_SPI0_CS1_PUDEN_SHIFT)    |
                   (1 << CONTROL_CONF_SPI0_CS1_CONF_SPI0_CS1_PUTYPESEL_SHIFT)|
                   (1 << CONTROL_CONF_SPI0_CS1_CONF_SPI0_CS1_RXACTIVE_SHIFT));
}


int enableClocks(void)
{
    unsigned int timeout;

    /*writing to CM_PER_MMC0_CLKCTRL enables SD OCP clock and CLKADPI clock*/
    REG32_write_masked(CM_PER_BASE, CM_PER_MMC0_CLKCTRL, 0b11, 0x2);
    timeout = 100000;

    /*wait until fully enabled*/
    while (REG32_read_masked(CM_PER_BASE, CM_PER_MMC0_CLKCTRL, (0b11 << 16)) 
            != 0x0)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC module clock\n");
            return -1;
        }
    }
    return 0;
}

int enableDeBounceClock(void)
{
    unsigned int timeout;

    /*we first need to enable the debounce clock in order to monitor if
     * software reset is done*/
    REG32_write_masked(CONTROL_MODULE_BASE, CLK32KDIVRATIO_CTRL, 0b1, 0b1);
    timeout = 100000;
    while(REG32_read_masked(CONTROL_MODULE_BASE, CLK32KDIVRATIO_CTRL, 0b1) 
            != 0b1)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for de bounce clock\n");
            return -1;
        }
    }
    return 0;
}


int softwareReset(void)
{
    unsigned int timeout;

    /*software reset*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, 0x2, 0x2); /*write 1 to 
    the SOFTRESET (1st) bit*/

    /*wait for the request to complete*/
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSSTATUS, 0x1) != 0x1)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC SOFTRESET\n");
            return -1;
        }
    }

    return 0;
}

int linesReset(unsigned int LINE_SHIFT)
{
    unsigned int timeout;

    uart_puts("waiting for lines reset\n");

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << LINE_SHIFT), 
            (0b1 << LINE_SHIFT));
   
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << LINE_SHIFT)) != 0)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for lines reset\n");
            return -1;
        }
    }
    return 0;
}


void setBusSupportedVoltage(void)
{
    /*support 3.3 voltage which is what beaglebone black supports according to
     *its documentation*/
    REG32_write_masked(MMCHS0_BASE, SD_CAPA, (0b111 << 24), (0b100 << 24));
    /*mask to access 26th 25th and 24th bit and writing 0 0 1
     *because we only support 3.3v not 1.8 or 3.0*/
    
    /*write max current 0 to register SD_CUR_CAPA for all 3 voltage*/
    /*REG32_write_masked(MMCHS0_BASE, SD_CUR_CAPA, (0b111111111111111111111111),
            0);*/
}

/*
 * 0x7 for 3.3v
 * */
void setBusVoltage(unsigned int voltage)
{
    /*SDVS bits 9-11 = 7h for 3.3v*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, (0b111 << 9), (voltage << 9));
}


void setBusWidth(void)
{
    /*set bus width to 1- bit*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, (0b1 << 1), 0);
}

int setBusPowerOn(void)
{
    unsigned int timeout;

    /*SDBP bit 8 1h = bus power on*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, (0b1 << 8), (0x1 << 8));

    timeout = 100000000;
    while(REG32_read_masked(MMCHS0_BASE, SD_HCTL, (0b1 << 8)) != (0x1 << 8))
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for SD bus power on\n");
            return -1;
        }
    }
    return 0;
}

int changeInternalClockFreq(unsigned int divider)
{
    unsigned int timeout;

    if(divider == 0 || divider == 1 || divider > 1023)
    {
        uart_puts("not supported frequency\n");
        return -1;
    }

    /*enable the internal clock*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1), (0x1));

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1111111111 << 6), 
            (divider << 6));

    /*wait for clock to stabilize by reading ICS bit of SD_SYSCTL*/
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 1)) != (0x1 << 1))
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC clock to be stable\n");
            return -1;
        }
    }
    return 0;
}

void sendInitStream(void)
{
    volatile int i;
    volatile unsigned int timeout;

    /*enable command completed interrupt*/
    REG32_write_masked(MMCHS0_BASE, SD_IE, 0b1, 1);

    /*enable clock timeout interrupt*/
    REG32_write_masked(MMCHS0_BASE, SD_IE, (0b1 << 16), (1 << 16));

    /*send initilization stream*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 1), (0x1 << 1));
    
    /*write 0x00000000 in the SD_CMD register*/
    REG32_write(MMCHS0_BASE, SD_CMD, 0x00000000);


    /*wait for init stream completion*/
    while(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1)) != 1)
    {
        uart_puts("waiting\n");
    }


    uart_puts("waiting\n");

    /*wait*/
    timeout = 1000000;
    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);
    
    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);

    for(i = 0; i < timeout; i++);

    /*blink LEDs*/
    LED_DELAY(1);
    
    uart_puts("wait complete\n");

    /*Set SD_STAT[0] CC bit to 0x1 to clear the flag*/
    REG32_write_masked(MMCHS0_BASE, SD_STAT, 0b1, 1);

    uart_puts("ending initializing sequence\n");

    /*Set SD_CON[1] INIT bit to 0x0 to end the initialization sequence*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 1), (0x0 << 1));

    /*Clear SD_STAT register (write 0xFFFFFFFF)*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF); 

}

/*function to send command to the card*/
int sendCommand(unsigned int cmdNum, unsigned int args, unsigned int rspType, 
        unsigned int response[])
{
    unsigned int reg;

    /*wait until issuing a command is allowed*/
    while(REG32_read_masked(MMCHS0_BASE, SD_PSTATE, (0b1)) != 0);

    /*clear status register*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

    /*write args*/
    REG32_write(MMCHS0_BASE, SD_ARG, args);
    
    reg = 0;
    reg |= (rspType << 16);
    reg |= (0b11 << 19);
    reg |= (cmdNum << 24);
    reg |= (0b00 << 22);

    if(cmdNum == CMD17)
    {
        /*
        uart_puts("sending CMD17\n");
        */

        reg |= (0x1 << 21); /*data present*/
        reg |= (0x1 << 4);  /*Data read*/
    }

    /*write command*/
    /*REG32_write_masked(MMCHS0_BASE, SD_CMD, (0b111111 << 24),
            (cmdNum << 24));*/
    REG32_write(MMCHS0_BASE, SD_CMD, reg);
    uart_puts("waiting for command to complete\n");

    /*wait for command to complete*/
    while(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1)) != 0x1)
    {
        uart_puts("SD_STAT: ");
        print_number(REG32_read(MMCHS0_BASE, SD_STAT), 2, 0);
        uart_puts("\n");
        /*check for error bits*/

        /*check for command timeout error*/
        if(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1 << 16)) == (0x1 << 16))
        {
            REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

            uart_puts("Command Timeout error\n");
            
            return CTO_ERROR;
        }

        /*can add other error checks*/
    }
    uart_puts("command complete STAT: ");
    unsigned int r = REG32_read(MMCHS0_BASE, SD_STAT);

    print_number(r, 2, 0);

    uart_puts("\n");

    /*clear the command complete flag*/
    if(cmdNum != CMD17)
    {
        REG32_write_masked(MMCHS0_BASE, SD_STAT, (0b1 << 0), (1 << 0));
    }

    uart_puts("after CC flag cleared STAT: ");
    r = REG32_read(MMCHS0_BASE, SD_STAT);

    print_number(r, 2, 0);

    uart_puts("\n");


    switch(cmdNum)
    {
        /*commands that have response R1*/
        /*case CMD0:*/
        case CMD1:
        case CMD6: 
        case CMD9:
        case CMD10:
        case CMD16:
        case CMD17:
        case CMD18:
        case CMD24:
        case CMD25:
        case CMD27:
        case CMD30:
        case CMD32:
        case CMD33:
        case CMD42:
        case CMD55:
        case CMD56:
        case CMD59:
        case ACMD41:
            response[0] = REG32_read(MMCHS0_BASE, SD_RSP10);
            break;
        
        /*case R1b*/
        case CMD12:
        case CMD28:
        case CMD29:
        case CMD38:
            response[0] = REG32_read(MMCHS0_BASE, SD_RSP10);
            break;

        /*case R2*/
        case CMD13:
            response[0] = REG32_read(MMCHS0_BASE, SD_RSP10);
            response[1] = REG32_read(MMCHS0_BASE, SD_RSP32);
            response[2] = REG32_read(MMCHS0_BASE, SD_RSP54);
            response[3] = REG32_read(MMCHS0_BASE, SD_RSP76);
            break;

        /*case R3*/
        case CMD58:
            response[0] = REG32_read(MMCHS0_BASE, SD_RSP10);
            break;

        /*case R7*/
        case CMD8:
            /*not specified which register to read R7 type of response from so
             * assuming from SD_RSP10*/
            response[0] = REG32_read(MMCHS0_BASE, SD_RSP10);
            break;

        /*no response*/
        case CMD0:
            break;

        default:
            uart_puts("command type not listed\n");
            return -1;
    }

    return CC;
}


int sdInitSeq()
{
    int n;
    int i;
    int timeout;
    int responses[4];
    
    n = 500;
    while(n > 0)
    {

        uart_puts("sending CMD0\n");

        /*send cmd0, no args, no response (CRC enabled by default)*/
        if(sendCommand(CMD0, 0x0, 0x0, responses) != CC)
        {
            uart_puts("problem with CMD0\n");
        }

        uart_puts("delay\n");

        LED_DELAY(1);

        uart_puts("sending CMD8\n");

        /*if(sendCommand(CMD8, (0x1 << 8) | (0x55), 0x2, responses) == CTO_ERROR)*/
        if(sendCommand(CMD8, (0xAA) | (0x000100u), 0x2u, responses) == CTO_ERROR)
        {
            uart_puts("CMD8 CTO\n");
        }
        else
        {
            uart_puts("CMD8 CC?\n");
        }

        return 0;
        
        if(sendCommand(CMD55, 0x0 << 16, 0x2, responses) == CTO_ERROR)
        {
            uart_puts("CMD55 CTO\n");
        }
        else
        {
            uart_puts("CMD55 CC?");
        }

        if(sendCommand(ACMD41, 0x40000000, 0x0, responses) == CC)
        {
            uart_puts("ACMD41 CC\n");
        }
        else
        {
            uart_puts("ACMD41 CTO?");
        }

        n--;
        continue;

        /*send cmd8, arg: , response 1 followed by arg*/
        /*
        if(sendCommand(CMD8, (0x1 << 8) | (0x55), 0x2, responses) != CC)
        {
            uart_puts("problem with CMD8\n");
            n--;
            continue;
        }

        uart_puts("test print number: ");

        print_number(11, 2, 0);

        uart_puts("\nCMD8 response: ");
*/
        /*read the response in responses[0]*/
        print_number(responses[0], 2, 0);
        
        uart_puts("\n");

        break;
    }

    return 0;
}

void mmc_controller_init_old(void)
{
    /*TRM: to initialize the MMC/SD/SDIO controller:
      • Initialize Clocks 
      • Software reset of the controller 
      • Set module's hardware capabilities 
      • Set module's Idle and Wake-Up modes*/
    
    uart_puts("mmc init!\n");

    uart_puts("PLLs init\n");

    
    CorePLLInit();
    PerPLLInit();

    /*pin muxing*/
    configure_mmc_pins();

    uart_puts("mmc pins configured\n");

    
    volatile unsigned int timeout;
    unsigned int divider;

    /*enable clocks*/ 
    uart_puts("trying to enable interface and functional clocks\n");

    if(enableClocks() != 0)
    {
        return;
    }

    uart_puts("Successfully enabled interface and functional clocks\n");
   
    /*
    if(enableDeBounceClock() != 0)
    {
        return;
    }
    */

    uart_puts("Successfully enabled debounce clock\n");

    if(softwareReset() != 0)
    {
        return;
    }

    if(linesReset(0) != 0)
    {
        return;
    }

    uart_puts("lines reset complete\n");

    uart_puts("SOFTRESET complete\n");

    /*Set modules hardware capabilitites*/

    /*MMC Host and Bus Configuration*/

    /*SD_CON register:*/
    
    /*OD bit not useful for SD card*/

    /*DW8 bit (5) must be cleared to 0 for SD/SDIO cards*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 5), (0 << 5));

    /*configure SD_CON to Standard MMC/SD/SDIO mode*/
    /*REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 12), (0 << 12));*/


    setBusSupportedVoltage();

    /*0x7 for 3.3v*/
    /*0x6 for 3.0v*/
    setBusVoltage(0x7);
    
    uart_puts("trying to power on SD bus\n");
    
    if(setBusPowerOn() != 0)
    {
        return;
    }

    uart_puts("set voltage is supported\n");


    if(changeInternalClockFreq(512) != 0)
    {
        return;
    }

    uart_puts("MMC clock stable after setting divider\n");
 
    /*Set module's Idle and Wake-Up modes*/

    /*CLOCKACTIVITY bits 8-9 3h = Interface and Functional clocks 
     * are maintained.*/
    /*REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b11 << 8), (0x3 << 8));
*/
    /*SIDLEMODE bits 3-4 1h = ignore idle request*/
    /*REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b11 << 3), (0x1 << 3));
*/

    /*set autoidle*/
    unsigned int reg = REG32_read(MMCHS0_BASE, SD_SYSCONFIG);

    reg |= (0x1 << 2) | (0x2 << 3) | (0x1);

    REG32_write(MMCHS0_BASE, SD_SYSCONFIG, reg);
    /*
    REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b1), (0x1));
    */

    uart_puts("MMC module init complete\n");

    /*Card Detection, Identification, and Selection*/
    
    sendInitStream();
    
    uart_puts("setting clock high\n");
    
    /*disable the internal clock*/
   /* 
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, 0b1, 0x0);
    */

    /*Change clock frequency to fit protocol*/
    /*CLKD bits 6-15 set divider to 1 to set clock freq to 24MHz from 
     * default 48000KHz*/
    
    /*divider = 96000 / 400;*/    /*or 2*/

    /*
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1111111111 << 6), 
            (divider << 6));
    */
    /*enable the internal clock*/
    
    /*
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, 0b1, 0x1);
    
    uart_puts("waiting for clock to be stable\n");
    */

    /*wait for clock to stabilize by reading ICS bit of SD_SYSCTL*/
    /*
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 1)) != (0x1 << 1))
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC clock to be stable\n");
            return;
        }
    }
    */
    uart_puts("now need to send commands\n");

    /*test SD card initialization sequence*/
    if(sdInitSeq() == 0)
    {
        uart_puts("SD card initialized\n");
        return;
    }

    return;

    /*Send a CMD0 command*/

    uart_puts("trying to send CMD0\n");

    unsigned int responses[4];

    if(sendCommand(CMD0, 0x0, 0x0 , responses) != CC)
    {
        return;
    }

    uart_puts("sent CMD0\n");

    uart_puts("trying to send CMD5\n");
   
    /*check for SDIO card since we have a SD card it should return CTO_ERROR*/
/*
    if(sendCommand(CMD5, 0x0, 0x0 , responses) != CTO_ERROR)
    {
        uart_puts("SDIO card?\n");
        return;
    }

    uart_puts("CMD5 sent\n");
  */  
    /*Set SD_SYSCTL[25] SRC bit to 0x1 and wait until it returns to 0x0*/
    uart_puts("waiting for Software reset for mmc_cmd line\n");

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 25), (0x1 << 25));

    timeout = 10000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 25)) != 
            (0x0 << 25))
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for mmc_cmd lines reset\n");
            return;
        }
    }

    uart_puts("lines reset complete\n");

    uart_puts("sending CMD8\n");

    if(sendCommand(CMD8, 0x0, 0b10, responses) != CTO_ERROR)
    {
        uart_puts("Card compliant with standard 2.0 or later\n");
        return;
    }

    /*Set SD_SYSCTL[25] SRC bit to 0x1 and wait until it returns to 0x0*/
    uart_puts("again waiting for Software reset for mmc_cmd line\n");

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 25), (0x1 << 25));

    timeout = 10000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 25)) != 
            (0x0 << 25))
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for mmc_cmd lines reset\n");
            return;
        }
    }

    uart_puts("lines reset complete\n");
    

    /*clear responses*/
    responses[0] = 0;

    uart_puts("trying to see if it is SD compliant with standard 1.x\n");

    timeout = 100000;

    while(REG32_read_masked(0x0, responses[0], (0b1 << 31)) != (0x1 << 31))
    {
        uart_puts("trying to send CMD55\n");

        if(sendCommand(CMD55, 0x0, 0x0 , responses) != CC)
        {
            return;
        }

        uart_puts("sent CMD55\n");

        uart_puts("sending ACMD41\n");

        if(sendCommand(ACMD41, 0x0, 0x0, responses) != CC)
        {
            uart_puts("handle MMC card case\n");
            return;
        }

        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for SD card standard compliant\n");
            return;
        }


    }

    uart_puts("it is SD compliant with standard 1.x\n");


    /*B*/
    uart_puts("trying to send CMD2\n");
   
    if(sendCommand(CMD2, 0x0, 0x0 , responses) != CC)
    {
        return;
    }

    uart_puts("CMD2 sent\n");
    
    uart_puts("trying to send CMD3\n");
   
    if(sendCommand(CMD3, 0x0, 0x0 , responses) != CC)
    {
        return;
    }

    uart_puts("CMD3 sent\n");

    uart_puts("assuming SD card because we know SD card\n");

    uart_puts("trying to send CMD7\n");
   
    if(sendCommand(CMD7, 0x0, 0x0 , responses) != CC)
    {
        return;
    }

    uart_puts("CMD7 sent\n");

    uart_puts("MMC init complete\n");
}


int enableInternalClock(void)
{
    unsigned int timeout;

    /*enable the internal clock*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1), (0x1));

    /*wait for clock to stabilize by reading ICS bit of SD_SYSCTL*/
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 1)) != (0x1 << 1))
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC clock to be stable\n");
            return -1;
        }
    }
    return 0;
}


void read_sector(unsigned int sector, uint8_t buffer[512])
{
    unsigned int responses[4];

    unsigned int stat, pstat, timeout;

    unsigned int i, word;

    /*set block size to 512 bytes*/
    REG32_write(MMCHS0_BASE, SD_BLK, 0x200);

    /*uart_puts("sending cmd16\n");

    sendCommand(CMD16, 512, 0x0, responses);*/

    uart_puts("sending cmd17\n");

    if(sendCommand(CMD17, sector, 0b10, responses) != CC)
    {
        uart_puts("CMD17 CTO\n");
        return;
    }

    timeout = 1000000;
    while(1)
    {
        stat = REG32_read(MMCHS0_BASE, SD_STAT);

        pstat = REG32_read(MMCHS0_BASE, SD_PSTATE);

        if(stat & (1 << 5) != 0)
        {
            uart_puts("buffer read ready\n");
            break;
        }

        if(stat == 0)
        {
            break;
        }

        uart_puts("SD_STAT: ");
        
        print_number(stat, 2, 0);

        uart_puts("\n");
        
        uart_puts("SD_PSTATE: ");
        
        print_number(pstat, 2, 0);

        uart_puts("\n");

        if(stat & (1 << 20) != 0)
        {
            uart_puts("Data timeout error\n");
            return;
        }

        if(stat & 0x78000 != 0)
        {
            uart_puts("Data error stat\n");
            return;
        }

        timeout--;

        if(timeout == 0)
        {
            uart_puts("Timeout waiting for BRR bit\n");
            return;
        }
    }

    for (i = 0; i < 128; i++)
    {
        word = REG32_read(MMCHS0_BASE, SD_DATA);
        buffer[i * 4]     = (uint8_t)(word & 0xFF);
        buffer[i * 4 + 1] = (uint8_t)((word >> 8) & 0xFF);
        buffer[i * 4 + 2] = (uint8_t)((word >> 16) & 0xFF);
        buffer[i * 4 + 3] = (uint8_t)((word >> 24) & 0xFF);
    }

    /*clear stat bits and disable data transfer*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

    REG32_write(MMCHS0_BASE, SD_CON, REG32_read(MMCHS0_BASE, SD_CON) 
            & !(1 << 1));
}


void mmc_controller_init(void)
{
    unsigned int timeout, reg;
   /* 
    CorePLLInit();

    PerPLLInit();
    */
    uart_puts("Enabling module clocks\n");

    /*
     * ENABLE CLOCK */
    if(enableClocks() != 0)
    {
        uart_puts("Failed to enable clocks\n");
        return;
    }
    
    uart_puts("Enabling internal clocks\n");
    
    if(enableInternalClock() != 0)
    {
        uart_puts("Failed to enable internal clocks\n");
        return;
    }

    /*maybe SD_HCTL bit 7 = 0?*/
    
    uart_puts("setting system config\n");

    /*disable autoidle*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b1), 0);
    /*check to make sure settings are applied*/
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b1)) != 0)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting to disable autoidle\n");
            return -1;
        }
    }
    
    uart_puts("setting interface and functional clocks to remain on\n");

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, 0b1, 0b1);

    uart_puts("setting power and clocks to remain on\n");
    
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 2), (0b1 << 2));
    
    uart_puts("setting MMC to never enter low power mode\n");

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0x000F0000), (0x000F0000));
    
    uart_puts("check to make sure settings applied\n");

    reg = REG32_read(MMCHS0_BASE, SD_SYSCTL);

    if(reg & (0b1) != (0b1)) 
    {
        uart_puts("Failed to set internal clock");
    }
    if(reg & (0b1 << 2) != (0b1 << 2))
    {
        uart_puts("Failed to set clock always on");
    }
    if(reg & (0x000F0000) != (0x000F0000))
    {
        uart_puts("Failed to set MMC to never enter low-power mode");
    }
    
    uart_puts("setting supported voltage\n");

    /*set bus voltage to 1.8*/
    setBusSupportedVoltage();


    setBusVoltage(0x5); /*5h = 1.8v*/

    uart_puts("setting and checking bus power\n");

    if(setBusPowerOn() != 0)
    {
        return;
    }

    uart_puts("disabling DDR\n");

    /*disable DDR*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0 << 19), 0);
    
    uart_puts("setting functional mode\n");

    REG32_write_masked(MMCHS0_BASE, SD_HCTL, 0, 0);
    
    uart_puts("setting block mode\n");

    /*set block size*/
    REG32_write(MMCHS0_BASE, SD_BLK, 0x200); /*0x200 for 512 bytes block size*/

    /*reset data and cmd lines*/
    
    uart_puts("resetting MMC_DAT line\n");

    /*26 for dat lines 25 for cmd line and 24 for all lines reset*/
    if(linesReset(26) != 0)
    {
        return;
    }

    uart_puts("resetting MMC_CMD line\n");

    if(linesReset(25) != 0)
    {
        return;
    }

    uart_puts("checking MMC0 clocks\n");

    /*check that clocks are on*/
    if(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 2)) != (0b1 << 2))
    {
        uart_puts("MMC clocks not on\n");
        return;
    }

    uart_puts("clearing all status flags\n");

    /*clear all status flags*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

    uart_puts("MMC init successful\n");

    return;
}
