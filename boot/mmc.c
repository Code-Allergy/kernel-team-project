#include <uart.h>
#include <utils.h>
#include <clock_module.h>
#include <mmc.h>


/*move this 2*/
#define CONTROL_MODULE_BASE 0x44E10000
#define CLK32KDIVRATIO_CTRL 0x444

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


/*function to send command to the card*/
int sendCommand(unsigned int cmdNum, unsigned int args, unsigned int flags, 
        unsigned int response[])
{
    /*wait until issuing a command is allowed*/
    while(REG32_read_masked(MMCHS0_BASE, SD_PSTATE, (0b11)) != 0);
    
    /*clear status register*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

    /*write args*/
    REG32_write(MMCHS0_BASE, SD_ARG, args);
 
    /*write flags*/
    REG32_write(MMCHS0_BASE, SD_CMD, flags);

    /*write command*/
    REG32_write_masked(MMCHS0_BASE, SD_CMD, (0b111111 << 24),
            (cmdNum << 24));

    uart_puts("waiting for command to complete\n");

    /*wait for command to complete*/
    while(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1)) != 0x1)
    {
        /*check for error bits*/
        
        /*check for command timeout error*/
        if(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1 << 16)) == 0x1)
        {
            uart_puts("Command Timeout error\n");
            
            return -1;
        }

    }
/*
    switch(cmdNum)
    {
        case :

            break;

        case R1b:
            break;
        
        case R2:
            break;

        case R3:
            break;

        case R7:
            break;

        default:
            uart_puts("response type should be specified\n");
            return -1;
    }

*/
    return 0;
}


void mmc_controller_init(void)
{
    uart_puts("mmc init!\n");

    /*pin muxing*/
    configure_mmc_pins();

    uart_puts("mmc pins configured\n");

    /*TRM: to initialize the MMC/SD/SDIO controller:
      • Initialize Clocks 
      • Software reset of the controller 
      • Set module's hardware capabilities 
      • Set module's Idle and Wake-Up modes*/
    
    unsigned int timeout;
    unsigned int divider;

    /*enable clocks*/ 
    uart_puts("trying to enable interface and functional clocks\n");

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
            return;
        }
    }

    uart_puts("Successfully enabled interface and functional clocks\n");
    
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
            return;
        }
    }
    
    uart_puts("Successfully enabled debounce clock\n");

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
            return;
        }
    }
    
    uart_puts("SOFTRESET complete\n");

    /*Set modules hardware capabilitites*/

    /*support 3.3 voltage which is what beaglebone black supports according to
     *its documentation*/
    REG32_write_masked(MMCHS0_BASE, SD_CAPA, (0b111 << 24), (0b001 << 24));
    /*mask to access 26th 25th and 24th bit and writing 0 0 1
     *because we only support 3.3v not 1.8 or 3.0*/
    
    /*write max current 0 to register SD_CUR_CAPA for all 3 voltage*/
    REG32_write_masked(MMCHS0_BASE, SD_CUR_CAPA, (0b111111111111111111111111),
            0);

    /*MMC Host and Bus Configuration*/

    /*SD_CON register:*/
    
    /*OD bit not useful for SD card*/

    /*DW8 bit (5) must be cleared to 0 for SD/SDIO cards*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 5), 0);

    /*configure SD_CON to Standard MMC/SD/SDIO mode*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 12), 0);

    /*SD_HCTL register:*/

    /*SDVS bits 9-11 = 7h for 3.3v*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, (0b111 << 9), (0x7 << 9));
    
    /*SDBP bit 8 1h = bus power on*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, (0b1 << 8), (0x1 << 8));

    /*set bus width to 1- bit*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, (0b1 << 1), 0);

    uart_puts("trying to power on SD bus\n");

    timeout = 100000000;
    while(REG32_read_masked(MMCHS0_BASE, SD_HCTL, (0b1 << 8)) != 0x1)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for SD bus power on\n");
            return;
        }
    }
    
    uart_puts("set voltage is supported\n");

    /*SD_SYSCTL register:*/

    /*enable the internal clock*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1), (0x1));

    /*CLKD bits 6-15 set divider to set clock freq to 80KHz from 48000KHz*/
    divider = 48000 / 80;   /*we want to set to 80 from 48MHz = 48000KHz = 
                              600*/

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1111111111 << 6), 
            (divider << 6));

    /*wait for clock to stabilize by reading ICS bit of SD_SYSCTL*/
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 1)) != 0x1)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC clock to be stable\n");
            return;
        }
    }

    uart_puts("MMC clock stable after setting divider\n");
 
    /*Set module's Idle and Wake-Up modes*/

    /*CLOCKACTIVITY bits 8-9 3h = Interface and Functional clocks 
     * are maintained.*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b11 << 8), (0x3 << 8));

    /*SIDLEMODE bits 3-4 1h = ignore idle request*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b11 << 3), (0x1 << 3));

    /*set autoidle*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCONFIG, (0b1), (0x1));

    uart_puts("MMC init complete\n");

    /*Card Detection, Identification, and Selection*/

    /*send initilization stream*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 1), (0x1 << 1));

    /*write 0x00000000 in the SD_CMD register*/
    REG32_write(MMCHS0_BASE, SD_CMD, 0x00000000);

    /*wait 1 ms*/
    timeout = 1000000;  /*CPU frequency is 1GHz = 1 billion instructions per 
                          second = 1 million instructions per milisecond*/
    while(--timeout >= 0);  /*this should take approximately 1ms*/

    /*Set SD_STAT[0] CC bit to 0x1 to clear the flag*/
    REG32_write_masked(MMCHS0_BASE, SD_STAT, 0b1, 0);

    /*Set SD_CON[1] INIT bit to 0x0 to end the initialization sequence*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 1), (0x0 << 1));

    /*Clear SD_STAT register (write 0xFFFFFFFF)*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);
    
    /*Change clock frequency to fit protocol*/
    /*CLKD bits 6-15 set divider to 1 to set clock freq to default 48000KHz*/
    divider = 1;    /*or 2*/

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1111111111 << 6), 
            (divider << 6));

    /*Send a CMD0 command*/

}
