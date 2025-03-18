#include <uart.h>
#include <utils.h>
#include <clock_module.h>
#include <mmc.h>


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
}

/*
 * 0x5 for 1.8v
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
        reg |= (0x1 << 21); /*data present*/
        reg |= (0x1 << 4);  /*Data read*/
    }

    /*write command*/
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


int mmc_read_sector(unsigned int sector, uint8_t buffer[512])
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
        return -1;
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
            return -1;
        }

        if(stat & 0x78000 != 0)
        {
            uart_puts("Data error stat\n");
            return -1;
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

    return 0;
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
            return;
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
