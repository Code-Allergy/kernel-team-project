#include <types.h>
#include <utils.h>
#include <ddr.h>
#include <uart.h>

#define TIMEOUT 1000000 /* Prevent infinite loops */

#define CONTROL_MODULE_BASE 0x44E10000U
#define CM_PER_BASE         0x44E00000U
#define CM_WKUP_BASE        0x44E00400U
#define SOC_EMIF_0_REGS     0x4C000000U
#define SOC_EMIF_0_REGS     0x4C000000U
#define DDR_PHY_CTRL_BASE   0x44E12000U

// /* DDR3-Specific Values */
#define DDR3_CMD0_SLAVE_RATIO_0         0x80U
#define DDR3_CMD1_SLAVE_RATIO_0         0x80U
#define DDR3_CMD2_SLAVE_RATIO_0         0x80U
#define DDR3_CMD0_INVERT_CLKOUT_0       0x00U
#define DDR3_CMD1_INVERT_CLKOUT_0       0x00U
#define DDR3_CMD2_INVERT_CLKOUT_0       0x00U
#define DDR3_DATA0_RD_DQS_SLAVE_RATIO_0 0x38U

// MINEE
#define CLKCTRL_MODULEMODE                0x3U
#define CLKCTRL_MODULEMODE_ENABLE         0x2U

#define CM_PER_EMIF_CLKCTRL 0x28
#define CM_PER_L3_CLKSTCTRL 0x0C

#define CM_DDR_CKE 0x131C
#define CM_DDR_CKE_CTRL_DDR_CKE_CTRL 0x00000001U
#define CM_CONTROL_EMIT_SDRAM_CONFIG 0x110

#define CM_IDLEST_DPLL_DDR 0x34
#define CM_CLKMODE_DPLL_DDR 0x94
#define DPLL_LOCK 0x07
#define CM_DIV_M2_DPLL_DDR 0xA0

// SOC_EMIF_0_REGS
#define EMIF_MOD_ID_REV 0x00U
#define STATUS 0x04U
#define SDRAM_CONFIG 0x08U
#define SDRAM_CONFIG_2 0x0CU
#define SDRAM_REF_CTRL 0x10U
#define SDRAM_REF_CTRL_SHDW 0x14U
#define SDRAM_TIM_1 0x18U
#define SDRAM_TIM_1_SHDW 0x1CU
#define SDRAM_TIM_2 0x20U
#define SDRAM_TIM_2_SHDW 0x24U
#define SDRAM_TIM_3 0x28U
#define SDRAM_TIM_3_SHDW 0x2CU

#define OCP_CFG_VAL_1 0x58U
#define OCP_CFG_VAL_2 0x5CU

#define ZQ_CONFIG 0xC8U

#define ST_DPLL_CLK 0x1

/* EMIF Power and Clock Status Flags */
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK 0x00000004U
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK   0x00000010U

#define PRCM_L3_PWR_CTRL 0x44E00000U

#define CM_WKUP_CM_CLKSEL_DPLL_DDR_DPLL_MULT 0x0007FF00
#define CM_WKUP_CM_CLKSEL_DPLL_DDR_DPLL_DIV  0x0000007F

static inline void enable_emif_clocks(void)
{
    uint32_t timeout = TIMEOUT;
    // L3 clock
    REG32_write_masked(CM_PER_BASE,
                       0xe0,
                       CLKCTRL_MODULEMODE_ENABLE,
                       CLKCTRL_MODULEMODE_ENABLE);
    WAIT_FOR_REG32(CM_PER_BASE,
                   0xe0,
                   CLKCTRL_MODULEMODE,
                   CLKCTRL_MODULEMODE_ENABLE,
                   timeout);

    // L4LS
    REG32_write_masked(CM_PER_BASE,
                       0x60,
                       CLKCTRL_MODULEMODE_ENABLE,
                       CLKCTRL_MODULEMODE_ENABLE);
    WAIT_FOR_REG32(CM_PER_BASE,
                   0x60,
                   CLKCTRL_MODULEMODE,
                   CLKCTRL_MODULEMODE_ENABLE,
                   timeout);

    // L4FW
    REG32_write_masked(CM_PER_BASE,
                       0x64,
                       CLKCTRL_MODULEMODE_ENABLE,
                       CLKCTRL_MODULEMODE_ENABLE);
    WAIT_FOR_REG32(CM_PER_BASE,
                   0x64,
                   CLKCTRL_MODULEMODE,
                   CLKCTRL_MODULEMODE_ENABLE,
                   timeout);

    // L4WKUP
    REG32_write_masked(CM_WKUP_BASE,
                       0xc,
                       CLKCTRL_MODULEMODE_ENABLE,
                       CLKCTRL_MODULEMODE_ENABLE);
    WAIT_FOR_REG32(CM_WKUP_BASE,
                   0xc,
                   CLKCTRL_MODULEMODE,
                   CLKCTRL_MODULEMODE_ENABLE,
                   timeout);

    // L3 instr
    REG32_write_masked(CM_PER_BASE,
                       0xdc,
                       CLKCTRL_MODULEMODE_ENABLE,
                       CLKCTRL_MODULEMODE_ENABLE);
    WAIT_FOR_REG32(CM_PER_BASE,
                   0xdc,
                   CLKCTRL_MODULEMODE,
                   CLKCTRL_MODULEMODE_ENABLE,
                   timeout);

    // L4HS
    REG32_write_masked(CM_PER_BASE,
                       0x120,
                       CLKCTRL_MODULEMODE_ENABLE,
                       CLKCTRL_MODULEMODE_ENABLE);
    WAIT_FOR_REG32(CM_PER_BASE,
                   0x120,
                   CLKCTRL_MODULEMODE,
                   CLKCTRL_MODULEMODE_ENABLE,
                   timeout);
}

#define CONTROL_VTP_CTRL_ENABLE 0x40
#define CONTROL_VTP_CTRL_READY  0x20
#define CONTROL_VTP_CTRL_CLRZ   0x01

static inline void configure_ddr_phy(void)
{
    uart_puts("Configuring DDR PHY...\n");
    /* Enable VTP and perform CLRZ sequence for calibration */
    REG32_write_masked(CONTROL_MODULE_BASE,
                       0xe0c,
                       CONTROL_VTP_CTRL_ENABLE,
                       CONTROL_VTP_CTRL_ENABLE);

    REG32_write_masked(CONTROL_MODULE_BASE, 0xe0c, 0x01, 0x01);

    while ((REG32_read_masked(
               CONTROL_MODULE_BASE, 0xe0c, CONTROL_VTP_CTRL_READY)) !=
           CONTROL_VTP_CTRL_READY)
    {
    }
    uart_puts("Done init DDR_PHY");
}

static inline void configure_ddr_phy_cmd_data(void)
{
    uart_puts("Configuring DDR PHY CMD and Data registers...\n");
    /* DDR PHY CMD0 configuration */
    // *(volatile uint32_t *)CMD0_REG_PHY_CTRL_SLAVE_RATIO_0 = 0x80;
    // *(volatile uint32_t *)CMD0_REG_PHY_DLL_LOCK_DIFF_0    = 0x1;
    // *(volatile uint32_t *)CMD0_REG_PHY_INVERT_CLKOUT_0      = 0x0;
    REG32_write(DDR_PHY_CTRL_BASE, 0x1C, 0x80);
    REG32_write(DDR_PHY_CTRL_BASE, 0x2C, DDR3_CMD0_INVERT_CLKOUT_0);

    /* DDR PHY CMD1 configuration */
    // *(volatile uint32_t *)CMD1_REG_PHY_CTRL_SLAVE_RATIO_0 = 0x80;
    // *(volatile uint32_t *)CMD1_REG_PHY_DLL_LOCK_DIFF_0    = 0x1;
    // *(volatile uint32_t *)CMD1_REG_PHY_INVERT_CLKOUT_0      = 0x0;
    REG32_write(DDR_PHY_CTRL_BASE, 0x50, 0x80);
    REG32_write(DDR_PHY_CTRL_BASE, 0x60, DDR3_CMD1_INVERT_CLKOUT_0);

    /* DDR PHY CMD2 configuration */
    // *(volatile uint32_t *)CMD2_REG_PHY_CTRL_SLAVE_RATIO_0 = 0x80;
    // *(volatile uint32_t *)CMD2_REG_PHY_DLL_LOCK_DIFF_0    = 0x1;
    // *(volatile uint32_t *)CMD2_REG_PHY_INVERT_CLKOUT_0      = 0x0;
    REG32_write(DDR_PHY_CTRL_BASE, 0x84, 0x80);
    REG32_write(DDR_PHY_CTRL_BASE, 0x94, DDR3_CMD2_INVERT_CLKOUT_0);

    /* DDR PHY Data Macro 0 configuration */
    // *(volatile uint32_t *)DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0   = 0x40;
    // *(volatile uint32_t *)DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0   = 0x00;
    // *(volatile uint32_t *)DATA0_REG_PHY_WRLVL_INIT_RATIO_0     = 0x0;
    // *(volatile uint32_t *)DATA0_REG_PHY_GATELVL_INIT_RATIO_0     = 0x0;
    // *(volatile uint32_t *)DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0    = 0x7B;
    // *(volatile uint32_t *)DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0    = 0x80;
    // *(volatile uint32_t *)DATA0_REG_PHY_DLL_LOCK_DIFF_0        = 1;
    REG32_write(DDR_PHY_CTRL_BASE, 0xC8, DDR3_DATA0_RD_DQS_SLAVE_RATIO_0);
    REG32_write(DDR_PHY_CTRL_BASE, 0xDC, 0x44);
    REG32_write(DDR_PHY_CTRL_BASE, 0x108, 0x94);
    REG32_write(DDR_PHY_CTRL_BASE, 0x120, 0x7D);

    /* DDR PHY Data Macro 1 configuration */
    // *(volatile uint32_t *)DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0   = 0x40;
    // *(volatile uint32_t *)DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0   = 0x00;
    // *(volatile uint32_t *)DATA1_REG_PHY_WRLVL_INIT_RATIO_0     = 0x0;
    // *(volatile uint32_t *)DATA1_REG_PHY_GATELVL_INIT_RATIO_0     = 0x0;
    // *(volatile uint32_t *)DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0    = 0x7B;
    // *(volatile uint32_t *)DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0    = 0x80;
    // *(volatile uint32_t *)DATA1_REG_PHY_DLL_LOCK_DIFF_0        = 1;
    REG32_write(DDR_PHY_CTRL_BASE, 0x170, 0x38);
    REG32_write(DDR_PHY_CTRL_BASE, 0x184, 0x44);
    REG32_write(DDR_PHY_CTRL_BASE, 0x1B0, 0x94);
    REG32_write(DDR_PHY_CTRL_BASE, 0x1C8, 0x7D);
    uart_puts("DDR PHY CMD and Data registers configured.\n");
}

static inline void configure_ddr_io(void)
{
    uint32_t reg;
    // data io ctl
    REG32_write(CONTROL_MODULE_BASE, 0x1404, 0x18B);
    REG32_write(CONTROL_MODULE_BASE, 0x1408, 0x18B);
    REG32_write(CONTROL_MODULE_BASE, 0x140C, 0x18B);
    REG32_write(CONTROL_MODULE_BASE, 0x1440, 0x18B);
    REG32_write(CONTROL_MODULE_BASE, 0x1444, 0x18B);

    // io ctl
    REG32_write_masked(CONTROL_MODULE_BASE, 0xe04, !0xefffffff, 0);

    // cke ctl
    REG32_write_masked(CONTROL_MODULE_BASE,
                       CM_DDR_CKE,
                       CM_DDR_CKE_CTRL_DDR_CKE_CTRL,
                       CM_DDR_CKE_CTRL_DDR_CKE_CTRL);

    REG32_write(SOC_EMIF_0_REGS, 0xE4, 0x06);
    REG32_write_masked(SOC_EMIF_0_REGS, 0xE4, 0x00100000, 0x00100000);

    REG32_write(SOC_EMIF_0_REGS, 0xE8, 0x6);
    REG32_write_masked(SOC_EMIF_0_REGS, 0xE8, 0x00100000, 0x00100000);
}

static inline void enable_core_pll(void)
{
    REG32_write_masked(CM_WKUP_BASE, 0x90, 0x00000007, 0x4);
    while (REG32_read_masked(CM_WKUP_BASE, 0x5c, 0x00000100) != 0x00000100) {}

    // set the multiplier and divider
    REG32_write(CM_WKUP_BASE, 0x68, (1000 << 0x00000008) | (23 << 0x00000000));

    // Set M4, M5, M6
    REG32_write_masked(CM_WKUP_BASE, 0x80, 0x0000001F, 10 << 0x00000000);
    REG32_write_masked(CM_WKUP_BASE, 0x84, 0x0000001F, 8 << 0x00000000);
    REG32_write_masked(CM_WKUP_BASE, 0xd8, 0x0000001F, 4 << 0);

    // LOCK the PLL
    REG32_write_masked(CM_WKUP_BASE, 0x90, DPLL_LOCK, DPLL_LOCK);
    while (REG32_read_masked(CM_WKUP_BASE, 0x5c, 0x00000001) != 0x00000001) {}
}

static inline void enable_ddr_pll(void)
{
    REG32_write_masked(CM_WKUP_BASE, 0x94, 0x7, 0x4);
    WAIT_FOR_REG32(CM_WKUP_BASE, 0x34, 0x100, 0x100, TIMEOUT);

    REG32_write_masked(CM_WKUP_BASE,
                       0x40,
                       CM_WKUP_CM_CLKSEL_DPLL_DDR_DPLL_MULT |
                           CM_WKUP_CM_CLKSEL_DPLL_DDR_DPLL_DIV,
                       (303 << 0x00000008) | (23 << 0x00000000));

    REG32_write_masked(CM_WKUP_BASE, 0xa0, 0x0000001F, 0x1);

    /* Lock the PLL and wait for it */
    REG32_write_masked(CM_WKUP_BASE, CM_CLKMODE_DPLL_DDR, DPLL_LOCK, DPLL_LOCK);
    WAIT_FOR_REG32(CM_WKUP_BASE, CM_IDLEST_DPLL_DDR, ST_DPLL_CLK, ST_DPLL_CLK, TIMEOUT);
}

static inline void init_emif(void)
{
    uint32_t expected;
    REG32_write_masked(
        CM_PER_BASE, CM_PER_EMIF_CLKCTRL,
        CLKCTRL_MODULEMODE, CLKCTRL_MODULEMODE_ENABLE);

    expected = CM_PER_L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK |
        CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK;
    WAIT_FOR_REG32(CM_PER_BASE, CM_PER_L3_CLKSTCTRL, expected, expected, TIMEOUT);
}

static inline void final_ddr3_init(void)
{
    /* TIM_1 */
    REG32_write(SOC_EMIF_0_REGS, SDRAM_TIM_1, 0x0AAAD4DB);
    REG32_write(SOC_EMIF_0_REGS, SDRAM_TIM_1_SHDW, 0x0AAAD4DB);
    /* TIM_2 */
    REG32_write(SOC_EMIF_0_REGS, SDRAM_TIM_2, 0x266B7FDA);
    REG32_write(SOC_EMIF_0_REGS, SDRAM_TIM_2_SHDW, 0x266B7FDA);
    /* TIM_3 */
    REG32_write(SOC_EMIF_0_REGS, SDRAM_TIM_3, 0x501F867F);
    REG32_write(SOC_EMIF_0_REGS, SDRAM_TIM_3_SHDW, 0x501F867F);
    /* REF control */
    REG32_write(SOC_EMIF_0_REGS, SDRAM_REF_CTRL, 0x00000C30);
    REG32_write(SOC_EMIF_0_REGS, SDRAM_REF_CTRL_SHDW, 0x00000C30);
    /* ZQ_CONFIG */
    REG32_write(SOC_EMIF_0_REGS, ZQ_CONFIG, 0x50074BE4);
    REG32_write(SOC_EMIF_0_REGS, SDRAM_CONFIG, 0x61C04BB2);

    /* Export SDRAM config register to the EMIF */
    REG32_write(CONTROL_MODULE_BASE, CM_CONTROL_EMIT_SDRAM_CONFIG, 0x61C00BB2);
}

void setup_memory(void)
{
    uart_puts("Starting to init mem!\n");

    enable_core_pll();

    enable_ddr_pll();

    enable_emif_clocks();
    init_emif();
    configure_ddr_phy(); // init_vtp
    configure_ddr_phy_cmd_data();
    configure_ddr_io();
    final_ddr3_init();
    uart_puts("Finished initializing memory\n");
}

int test_ddr3_memory(void)
{
    uint32_t* mem_addr;
    uint32_t patterns[] = {0xAAAAAAAA, 0x55555555, 0x00000000, 0xFFFFFFFF};
    int errors          = 0;
    int p;

    for (p = 0; p < 4; p++)
    {
        uint32_t test_pattern = patterns[p];

        for (mem_addr = (uint32_t*) DDR_START_ADDR;
             mem_addr < (uint32_t*) (DDR_START_ADDR + DDR_SIZE);
             mem_addr++)
        {
            *mem_addr          = test_pattern;
            uint32_t read_back = *mem_addr;

            if (read_back != test_pattern)
            {
                errors++;
            }
        }

        uart_puts("DDR3 Memory Test Mid\n");
    }

    if (errors > 0)
    {
        uart_puts("Memory Test Failed!\n");
        // uart_print_hex(errors);
        // uart_puts("\n");
    }
    else
    {
        uart_puts("Memory Test Passed!\n");
    }

    return 0;
}

// void itoa(int num, char* str, int base)
// {
//     int i          = 0;
//     int isNegative = 0;

//     /* Handle 0 explicitly */
//     if (num == 0)
//     {
//         str[i++] = '0';
//         str[i]   = '\0';
//         return;
//     }

//     /* Handle negative numbers for base 10 */
//     if (num < 0 && base == 10)
//     {
//         isNegative = 1;
//         num        = -num;
//     }

//     /* Manually convert number to string (without / or %) */
//     while (num > 0)
//     {
//         int quotient = 0, remainder = num;

//         /* Manual division (bitwise shifting for powers of 2) */
//         while (remainder >= base)
//         {
//             remainder -= base;
//             quotient++;
//         }

//         str[i++] = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
//         num      = quotient; /* Move to next digit */
//     }

//     /* Add '-' sign if negative */
//     if (isNegative)
//         str[i++] = '-';

//     str[i] = '\0';

//     /* Reverse the string */
//     int start = 0, end = i - 1;
//     while (start < end)
//     {
//         char temp  = str[start];
//         str[start] = str[end];
//         str[end]   = temp;
//         start++;
//         end--;
//     }
// }

// void uart_print_hex(uint32_t value)
// {
//     char
//         buffer[12]; /* Enough space for "0x" + 8 hex digits + null terminator */
//     buffer[0] = '0';
//     buffer[1] = 'x';
//     buffer[2] = '\0';

//     itoa(
//         value, buffer + 2, 16); /* Convert value to hex starting at buffer[2] */

//     uart_puts(buffer); /* Send hex string over UART */
//     uart_puts("\n");
// }
