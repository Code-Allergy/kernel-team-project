#include "interrupt.h"
#include <uart.h>
#include <utils.h>
#include <clock_module.h>
#include <mmc.h>

/*enable the interface and functional clocks
 * returns 0 on success and -1 on error*/
int enableClocks(void)
{
    unsigned int timeout;

    /*writing to CM_PER_MMC0_CLKCTRL enables SD OCP clock and CLKADPI clock*/
    REG32_write_masked(CM_PER_BASE, CM_PER_MMC0_CLKCTRL, 0x3, 0x2);
    timeout = 100000;

    /*wait until fully enabled*/
    while (REG32_read_masked(CM_PER_BASE, CM_PER_MMC0_CLKCTRL, (0x3 << 16)) !=
           0x0)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC module clock\n");
            return -1;
        }
    }
    return 0;
}

/*lines reset
 * LINE_SHIFT should be
 * 24 - reset for all
 * 25 - reset mmc_cmd line
 * 26 - reset mmc_dat line
 * returns 0 on success -1 on error*/
int linesReset(unsigned int LINE_SHIFT)
{
    unsigned int timeout;

    uart_puts("waiting for lines reset\n");

    REG32_write_masked(MMCHS0_BASE,
                       SD_SYSCTL,
                       (SD_SYSCTL_LINES_RESET << LINE_SHIFT),
                       (SD_SYSCTL_LINES_RESET << LINE_SHIFT));

    timeout = 100000;
    while (REG32_read_masked(MMCHS0_BASE,
                             SD_SYSCTL,
                             (SD_SYSCTL_LINES_RESET << LINE_SHIFT)) != 0)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for lines reset\n");
            return -1;
        }
    }
    return 0;
}

/*sets supported voltage Hardcoded to 1.8v*/
void setBusSupportedVoltage(void)
{
    /*support 3.3 voltage which is what beaglebone black supports according to
     *its documentation*/
    REG32_write_masked(MMCHS0_BASE, SD_CAPA, SD_CAPA_VS, SD_CAPA_VS_1P8);
}

/*sets bus voltage, voltage should be
 * 0x5 for 1.8v
 * 0x6 for 3.0v
 * 0x7 for 3.3v
 * */
void setBusVoltage(unsigned int voltage)
{
    REG32_write_masked(
        MMCHS0_BASE, SD_HCTL, SD_HCTL_SDVS, (voltage << SD_HCTL_SDVS_SHIFT));
}

/*set bus power on
 * returns 0 on success and -1 on error*/
int setBusPowerOn(void)
{
    unsigned int timeout;

    /*SDBP bit 8 1h = bus power on*/
    REG32_write_masked(MMCHS0_BASE, SD_HCTL, SD_HCTL_SDBP, SD_HCTL_SDBP_ON);

    timeout = 100000000;
    while (REG32_read_masked(MMCHS0_BASE, SD_HCTL, SD_HCTL_SDBP) !=
           SD_HCTL_SDBP_ON)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for SD bus power on\n");
            return -1;
        }
    }
    return 0;
}

/* function to send command to the card*
 * returns CC on Command Complete and CTO_ERROR for command timeout error */
int sendCommand(unsigned int cmdNum,
                unsigned int args,
                unsigned int rspType,
                unsigned int response[])
{
    unsigned int reg;

    /*wait until issuing a command is allowed*/
    while (REG32_read_masked(MMCHS0_BASE, SD_PSTATE, SD_PSTATE_CC) !=
           SD_PSTATE_C_ALLOWED)
        ;

    /*clear status register*/
    REG32_write(MMCHS0_BASE, SD_STAT, SD_STAT_CLEAR);

    /*write args*/
    REG32_write(MMCHS0_BASE, SD_ARG, args);

    reg = 0;
    reg |= (rspType << SD_CMD_RSPTYPE_SHIFT);
    reg |= SD_CMD_CCCE_CICE_ENABLE;
    reg |= (cmdNum << SD_CMD_INDX_SHIFT);
    reg |= SD_CMD_CMDTYPE_NORMAL;

    if (cmdNum == CMD17)
    {
        reg |= SD_CMD_DP;        /*data present*/
        reg |= SD_CMD_DDIR_READ; /*Data read*/
    }

    /* write command */
    log_message(LOG_LEVEL_DEBUG, "sending command %d\n", cmdNum);
    REG32_write(MMCHS0_BASE, SD_CMD, reg);
    log_message(LOG_LEVEL_DEBUG, "waiting for command to complete\n");

    /*wait for command to complete*/
    while (REG32_read_masked(MMCHS0_BASE, SD_STAT, SD_STAT_CC) != SD_STAT_CC)
    {
        /*check for command timeout error*/
        if (REG32_read_masked(MMCHS0_BASE, SD_STAT, SD_STAT_CTO) == SD_STAT_CTO)
        {
            REG32_write(MMCHS0_BASE, SD_STAT, SD_STAT_CLEAR);
            log_message(
                LOG_LEVEL_ERROR, "Command Timeout error on cmd %d\n", cmdNum);
            return CTO_ERROR;
        }

        /*can add other error checks*/
    }

    /*clear the command complete flag*/
    if (cmdNum != CMD17)
    {
        REG32_write_masked(
            MMCHS0_BASE, SD_STAT, SD_STAT_CC_CLEAR, SD_STAT_CC_CLEAR);
    }

    switch (cmdNum)
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

/* enable internal clock and wait for it to be stable
 * returns 0 on success and -1 on error */
int enableInternalClock(void)
{
    unsigned int timeout;

    /*enable the internal clock*/
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, SD_SYSCTL_ICE, SD_SYSCTL_ICE);

    /*wait for clock to stabilize by reading ICS bit of SD_SYSCTL*/
    timeout = 100000;
    while (REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, SD_SYSCTL_ICS) !=
           SD_SYSCTL_ICS)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting for MMC clock to be stable\n");
            return -1;
        }
    }
    return 0;
}

/* Reads physical sector `sector` and copies bytes into `buffer`,
 * returning 0 on success. */
int mmc_read_sector(unsigned int sector, uint8_t buffer[512])
{
    unsigned int responses[4];
    unsigned int stat, timeout;
    unsigned int i, word;

    if (sendCommand(CMD17, sector, SD_CMD_48BITRSP, responses) != CC)
    {
        log_message(LOG_LEVEL_WARN, "CMD17 CTO\n");
        return -1;
    }

    timeout = 1000000;
    /* Wait for SD_STAT to report it is finished with the read */
    while (1)
    {
        stat = REG32_read(MMCHS0_BASE, SD_STAT);
        if ((stat & (1 << 5)) != 0) /*check BRR bit*/
        {
            break;
        }

        if (stat == 0)
        {
            log_message(LOG_LEVEL_ERROR,
                        "No response from card (SD_STAT = 0 after cmd17)\n");
            break;
        }

        if ((stat & (1 << 20)) != 0)
        {
            log_message(LOG_LEVEL_ERROR, "Data timeout error after cmd17\n");
            return -1;
        }

        if ((stat & 0x78000) != 0)
        {
            log_message(LOG_LEVEL_ERROR,
                        "Some other error.."
                        "cmd17 stat raw: %d",
                        stat);
            return -1;
        }

        timeout--;

        if (timeout == 0)
        {
            log_message(LOG_LEVEL_ERROR, "Timeout waiting for BRR bit\n");
            return -1;
        }
    }

    for (i = 0; i < 128; i++)
    {
        word              = REG32_read(MMCHS0_BASE, SD_DATA);
        buffer[i * 4]     = (uint8_t) (word & 0xFF);
        buffer[i * 4 + 1] = (uint8_t) ((word >> 8) & 0xFF);
        buffer[i * 4 + 2] = (uint8_t) ((word >> 16) & 0xFF);
        buffer[i * 4 + 3] = (uint8_t) ((word >> 24) & 0xFF);
    }

    /*clear stat bits and disable data transfer*/
    REG32_write(MMCHS0_BASE, SD_STAT, SD_STAT_CC_CLEAR);
    REG32_write(
        MMCHS0_BASE, SD_CON, REG32_read(MMCHS0_BASE, SD_CON) & ~(1 << 1));

    return 0;
}

void mmc_controller_init(void)
{
    unsigned int timeout, reg;
    log_message(LOG_LEVEL_INFO, "init mmc controller\n");

    log_message(LOG_LEVEL_DEBUG, "Enabling module clocks\n");
    if (enableClocks() != 0)
    {
        uart_puts("Failed to enable clocks\n");
        return;
    }

    log_message(LOG_LEVEL_DEBUG, "Enabling internal clocks\n");
    if (enableInternalClock() != 0)
    {
        uart_puts("Failed to enable internal clocks\n");
        return;
    }

    /*maybe SD_HCTL bit 7 = 0?*/

    log_message(LOG_LEVEL_DEBUG, "setting system config\n");

    /*disable autoidle*/
    REG32_write_masked(MMCHS0_BASE,
                       SD_SYSCONFIG,
                       SD_SYSCONFIG_AUTOIDLE,
                       SD_SYSCONFIG_AUTOIDLE_DISABLE);

    /*check to make sure settings are applied*/
    timeout = 100000;
    while (
        REG32_read_masked(MMCHS0_BASE, SD_SYSCONFIG, SD_SYSCONFIG_AUTOIDLE) !=
        SD_SYSCONFIG_AUTOIDLE_DISABLE)
    {
        if (--timeout == 0)
        {
            uart_puts("Timeout waiting to disable autoidle\n");
            return;
        }
    }

    log_message(LOG_LEVEL_DEBUG,
                "setting interface and functional clocks to remain on\n");
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, SD_SYSCTL_ICE, SD_SYSCTL_ICE);

    log_message(LOG_LEVEL_DEBUG, "setting power and clocks to remain on\n");
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, SD_SYSCTL_CEN, SD_SYSCTL_CEN);

    log_message(LOG_LEVEL_DEBUG, "setting MMC to never enter low power mode\n");
    REG32_write_masked(MMCHS0_BASE, SD_SYSCTL, SD_SYSCTL_DTO, SD_SYSCTL_DTO);

    /* Verify that what we did had an effect */
    log_message(LOG_LEVEL_DEBUG, "check to make sure settings applied\n");
    reg = REG32_read(MMCHS0_BASE, SD_SYSCTL);
    if ((reg & SD_SYSCTL_ICE) != SD_SYSCTL_ICE)
    {
        uart_puts("Failed to set internal clock");
    }
    if ((reg & SD_SYSCTL_CEN) != SD_SYSCTL_CEN)
    {
        uart_puts("Failed to set clock always on");
    }
    if ((reg & SD_SYSCTL_DTO) != SD_SYSCTL_DTO)
    {
        uart_puts("Failed to set MMC to never enter low-power mode");
    }

    log_message(LOG_LEVEL_DEBUG, "setting supported voltage\n");

    /*set bus voltage to 1.8*/
    setBusSupportedVoltage();
    setBusVoltage(SD_HCTL_SDBV_1P8); /*5h = 1.8v*/

    log_message(LOG_LEVEL_DEBUG, "setting and checking bus power\n");

    if (setBusPowerOn() != 0)
    {
        return;
    }

    log_message(LOG_LEVEL_DEBUG, "disabling DDR\n");

    /*disable DDR*/
    REG32_write_masked(MMCHS0_BASE, SD_CON, SD_CON_DDR, SD_CON_DDR_DISABLE);

    log_message(LOG_LEVEL_DEBUG, "setting functional mode\n");

    REG32_write_masked(MMCHS0_BASE, SD_HCTL, 0, 0);

    /*reset data and cmd lines*/

    log_message(LOG_LEVEL_DEBUG, "resetting MMC_DAT line\n");

    /*26 for dat lines 25 for cmd line and 24 for all lines reset*/
    if (linesReset(SD_SYSCTL_SRD) != 0)
    {
        return;
    }

    log_message(LOG_LEVEL_DEBUG, "resetting MMC_CMD line\n");

    if (linesReset(SD_SYSCTL_SRC) != 0)
    {
        return;
    }

    /*check that clocks are on*/
    log_message(LOG_LEVEL_DEBUG, "checking MMC0 clocks\n");
    if (REG32_read_masked(MMCHS0_BASE, SD_SYSCTL, SD_SYSCTL_CEN) !=
        SD_SYSCTL_CEN)
    {
        uart_puts("MMC clocks not on\n");
        return;
    }

    /*clear all status flags*/
    log_message(LOG_LEVEL_DEBUG, "clearing all status flags\n");
    REG32_write(MMCHS0_BASE, SD_STAT, 0xFFFFFFFF);

    /*set block size to 512 bytes*/
    log_message(LOG_LEVEL_DEBUG,
                "setting block size to 512 bytes and 1 block transfer mode\n");
    REG32_write(MMCHS0_BASE, SD_BLK, 1 << 16 | SD_BLK_512);

    log_message(LOG_LEVEL_INFO, "MMC init successful\n");
}
