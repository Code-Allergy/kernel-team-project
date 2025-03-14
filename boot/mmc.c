#include <uart.h>
#include <utils.h>
#include <clock_module.h>
#include <mmc.h>


/*move this 2*/
#define CONTROL_MODULE_BASE 0x44E10000
#define CLK32KDIVRATIO_CTRL 0x444
/*SD card command timeout error*/
#define CTO_ERROR -16
#define CC 0

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
    uart_puts("send command called\n");
    /*wait until issuing a command is allowed*/
    while(REG32_read_masked(MMCHS0_BASE, SD_PSTATE, (0b1)) != 0);

    /*clear status register*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

    /*write args*/
    REG32_write(MMCHS0_BASE, SD_ARG, args);
 
    /*write flags*/
    /*
    REG32_write(MMCHS0_BASE, SD_CMD, flags);
    */

    /*write command*/
    REG32_write_masked(MMCHS0_BASE, SD_CMD, (0b111111 << 24),
            (cmdNum << 24));

    uart_puts("waiting for command to complete\n");

    /*wait for command to complete*/
    while(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1)) != 0x1)
    {
        /*check for error bits*/

        /*check for command timeout error*/
        if(REG32_read_masked(MMCHS0_BASE, SD_STAT, (0b1 << 16)) == (0x1 << 16))
        {
            uart_puts("Command Timeout error\n");
            
            return CTO_ERROR;
        }

        /*can add other error checks*/
    }
    uart_puts("command complete\n");
    
    switch(cmdNum)
    {
        /*commands that have response R1*/
        case CMD0:
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

        default:
            uart_puts("command type not listed\n");
            return -1;
    }

    return CC;
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

int linesReset(void)
{
    unsigned int timeout;

    uart_puts("waiting for lines reset\n");

    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 24), (0b1 << 24));
   
    timeout = 100000;
    while(REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, (0b1 << 24)) != 0)
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
    REG32_write_masked(MMCHS0_BASE, SD_CAPA, (0b111 << 24), (0b001 << 24));
    /*mask to access 26th 25th and 24th bit and writing 0 0 1
     *because we only support 3.3v not 1.8 or 3.0*/
    
    /*write max current 0 to register SD_CUR_CAPA for all 3 voltage*/
    REG32_write_masked(MMCHS0_BASE, SD_CUR_CAPA, (0b111111111111111111111111),
            0);
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

int enableInternalClock(unsigned int divider)
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
    /*send initilization stream*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 1), (0x1 << 1));
}

void mmc_controller_init(void)
{
    /*TRM: to initialize the MMC/SD/SDIO controller:
      • Initialize Clocks 
      • Software reset of the controller 
      • Set module's hardware capabilities 
      • Set module's Idle and Wake-Up modes*/
    
    uart_puts("mmc init!\n");

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
    
    if(enableDeBounceClock() != 0)
    {
        return;
    }

    uart_puts("Successfully enabled debounce clock\n");

    if(softwareReset() != 0)
    {
        return;
    }

    if(linesReset() != 0)
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
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 5), 0);

    /*configure SD_CON to Standard MMC/SD/SDIO mode*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 12), 0);


    setBusSupportedVoltage();

    /*0x7 for 3.3v*/
    setBusVoltage(0x7);
    
    uart_puts("trying to power on SD bus\n");
    
    if(setBusPowerOn() != 0)
    {
        return;
    }

    uart_puts("set voltage is supported\n");


    if(enableInternalClock(96000 / 400) != 0)
    {
        return;
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

    uart_puts("MMC module init complete\n");

    /*Card Detection, Identification, and Selection*/
    
    sendInitStream();

    /*write 0x00000000 in the SD_CMD register*/
    REG32_write(MMCHS0_BASE, SD_CMD, 0x00000000);
    
    uart_puts("waiting 1ms\n");

    /*wait 1 ms*/
    timeout = 1000000;  /*CPU frequency is set to 80KHz = 80 000 instructions 
                          per 
                          second = 80 instructions per milisecond
                          we have 1 mil / 2 because we sould have ~ 4 
                          instructions in this loop*/
    while(timeout > 0)
    {
        timeout--;
        /*
        uart_puts("waiting?\n");
        */
    }  /*this should take approximately 4ms+*/

    uart_puts("wait complete\n");

    /*Set SD_STAT[0] CC bit to 0x1 to clear the flag*/
    REG32_write_masked(MMCHS0_BASE, SD_STAT, 0b1, 0);

    uart_puts("ending initializing sequence\n");

    /*Set SD_CON[1] INIT bit to 0x0 to end the initialization sequence*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, (0b1 << 1), (0x0 << 1));

    /*Clear SD_STAT register (write 0xFFFFFFFF)*/
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);
    
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
    /*Send a CMD0 command*/

    uart_puts("trying to send CMD0\n");

    unsigned int responses[4];

    if(sendCommand(CMD0, 0x0, 0x0 , responses) != CC)
    {
        return;
    }

    uart_puts("sent CMD0\n");

    uart_puts("trying to send CMD5\n");
   
    /*checks for SDIO card since we have a SD card it should return CTO_ERROR*/
    if(sendCommand(CMD5, 0x0, 0x0 , responses) != CTO_ERROR)
    {
        uart_puts("SDIO card?\n");
        return;
    }

    uart_puts("CMD5 sent\n");
    
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

    if(sendCommand(CMD8, 0x0, 0x0, responses) != CTO_ERROR)
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
