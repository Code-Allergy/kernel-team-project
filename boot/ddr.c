#include <types.h> 
#include <ddr.h>
#include <uart.h>

#define TIMEOUT 1000000  /* Prevent infinite loops */

/* Register definitions for AM335x (BeagleBone Black) in C90 style */

/* DDR PLL Registers */
#define CM_CLKMODE_DPLL_DDR   0x44E00594
#define CM_IDLEST_DPLL_DDR    0x44E00520
#define CM_CLKSEL_DPLL_DDR    0x44E00598
#define CM_DIV_M2_DPLL_DDR    0x44E0059C

/* L3 Clock Domain Registers (CM_PER) */
#define CM_PER_L3_CLKSTCTRL   0x44E0000C
#define CM_PER_L3_CLKCTRL     0x44E00010

/* EMIF Clock Control Register */
#define CM_PER_EMIF_CLKCTRL   0x44E00028

/* VTP Control Register */
#define VTP_CTRL              0x44E10E0C

/* Core PLL Registers */
#define CM_CLKMODE_DPLL_CORE  0x44E00840
#define CM_CLKSEL_DPLL_CORE   0x44E00850
#define CM_IDLEST_DPLL_CORE   0x44E00864
#define CM_DIV_M4_DPLL_CORE   0x44E00868
#define CM_DIV_M5_DPLL_CORE   0x44E0086C
#define CM_DIV_M6_DPLL_CORE   0x44E00870

/* EMIF SDRAM Registers */
#define EMIF_SDRAM_CONFIG         0x4C000008
#define EMIF_SDRAM_CONFIG_2       0x4C00000C
#define EMIF_SDRAM_REF_CTRL       0x4C000010
#define EMIF_SDRAM_REF_CTRL_SHDW  0x4C000014
#define EMIF_SDRAM_TIM_1          0x4C000018
#define EMIF_SDRAM_TIM_1_SHDW     0x4C00001C
#define EMIF_SDRAM_TIM_2          0x4C000020
#define EMIF_SDRAM_TIM_2_SHDW     0x4C000024
#define EMIF_SDRAM_TIM_3          0x4C000028
#define EMIF_SDRAM_TIM_3_SHDW     0x4C00002C
#define EMIF_PWR_MGMT_CTRL        0x4C000038
#define EMIF_PWR_MGMT_CTRL_SHDW   0x4C00003C

/* EMIF DDR PHY Control Registers */
#define EMIF_DDR_PHY_CTRL_1       0x4C0000E4
#define EMIF_DDR_PHY_CTRL_1_SHDW  0x4C0000E8

/* EMIF SDRAM Status and Clock Control Mask */
#define EMIF_SDRAM_STATUS         0x4C00004C
#define EMIF_CLKCTRL_IDLEST_MASK  (0x3 << 16)

/* Base Addresses */
#define CM_PER_BASE         0x44E0000C
#define CONTROL_MODULE_BASE 0x44E10000
#define SOC_EMIF_0_REGS     0x44E00028

/* EMIF Clock Control Module Settings */
#define CM_PER_EMIF_FW_CLKCTRL               0x44E0002C
#define CM_PER_EMIF_CLKCTRL_MODULEMODE_ENABLE 0x2
#define CM_PER_EMIF_FW_CLKCTRL_MODULEMODE_ENABLE 0x2

/* EMIF Power and Clock Status Flags */
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK 0x00000002
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK   0x00000001

/* DDR PHY Control Registers */
#define CONTROL_VTP_CTRL          0x44E10E0C
#define CONTROL_VTP_CTRL_ENABLE   0x00000040
#define CONTROL_VTP_CTRL_CLRZ     0x00000020
#define CONTROL_VTP_CTRL_READY    0x00000080

/* DDR CMD IO Control Registers */
#define CONTROL_DDR_CMD_IOCTRL(n)  (0x44E10E30 + ((n) * 4))
#define CONTROL_DDR_DATA_IOCTRL(n) (0x44E10E60 + ((n) * 4))
#define CONTROL_DDR_IO_CTRL        0x44E10E90
#define CONTROL_DDR_CKE_CTRL       0x44E10E94
#define CONTROL_DDR_CKE_CTRL_DDR_CKE_CTRL 0x00000001

/* Additional EMIF Registers */
#define EMIF_DDR_PHY_CTRL_2        0x4C0000EC
#define EMIF_ZQ_CONFIG             0x4C0000C8
#define CONTROL_SECURE_EMIF_SDRAM_CONFIG 0x44E10E0C

/* DDR PHY Timing and Configuration Registers */
#define CMD0_SLAVE_RATIO_0   0x44E12000
#define CMD1_SLAVE_RATIO_0   0x44E12004
#define CMD2_SLAVE_RATIO_0   0x44E12008
#define CMD0_INVERT_CLKOUT_0 0x44E1200C
#define CMD1_INVERT_CLKOUT_0 0x44E12010
#define CMD2_INVERT_CLKOUT_0 0x44E12014

/* Data Slave Ratio Registers */
#define DATA0_RD_DQS_SLAVE_RATIO_0  0x44E12018
#define DATA0_WR_DQS_SLAVE_RATIO_0  0x44E1201C
#define DATA0_FIFO_WE_SLAVE_RATIO_0 0x44E12020
#define DATA0_WR_DATA_SLAVE_RATIO_0 0x44E12024
#define DATA1_RD_DQS_SLAVE_RATIO_0  0x44E12028
#define DATA1_WR_DQS_SLAVE_RATIO_0  0x44E1202C
#define DATA1_FIFO_WE_SLAVE_RATIO_0 0x44E12030
#define DATA1_WR_DATA_SLAVE_RATIO_0 0x44E12034

/* DDR3-Specific Values */
#define DDR3_CMD0_SLAVE_RATIO_0   0x80
#define DDR3_CMD1_SLAVE_RATIO_0   0x80
#define DDR3_CMD2_SLAVE_RATIO_0   0x80
#define DDR3_CMD0_INVERT_CLKOUT_0 0x00
#define DDR3_CMD1_INVERT_CLKOUT_0 0x00
#define DDR3_CMD2_INVERT_CLKOUT_0 0x00
#define DDR3_DATA0_RD_DQS_SLAVE_RATIO_0  0x38
#define DDR3_DATA0_WR_DQS_SLAVE_RATIO_0  0x00
#define DDR3_DATA0_FIFO_WE_SLAVE_RATIO_0 0x80
#define DDR3_DATA0_WR_DATA_SLAVE_RATIO_0 0x80
#define DDR3_DATA0_RD_DQS_SLAVE_RATIO_1  0x38
#define DDR3_DATA0_WR_DQS_SLAVE_RATIO_1  0x00
#define DDR3_DATA0_FIFO_WE_SLAVE_RATIO_1 0x80
#define DDR3_DATA0_WR_DATA_SLAVE_RATIO_1 0x80
#define DDR3_CONTROL_DDR_CMD_IOCTRL_0 0x18B
#define DDR3_CONTROL_DDR_CMD_IOCTRL_1 0x18B
#define DDR3_CONTROL_DDR_CMD_IOCTRL_2 0x18B
#define DDR3_CONTROL_DDR_DATA_IOCTRL_0 0x18B
#define DDR3_CONTROL_DDR_DATA_IOCTRL_1 0x18B
#define DDR3_CONTROL_DDR_IO_CTRL       0x18B
#define DDR3_CONTROL_DDR_CKE_CTRL      0x00000001
#define DDR3_EMIF_ZQ_CONFIG_VAL        0x50074BE4
#define DDR3_EMIF_SDRAM_CONFIG         0x61851B32
#define DDR3_EMIF_SDRAM_REF_CTRL_VAL1  0x00001035
#define DDR3_EMIF_SDRAM_REF_CTRL_SHDW_VAL1 0x00001035
#define DDR3_EMIF_DDR_PHY_CTRL_1       0x50000800
#define DDR3_EMIF_DDR_PHY_CTRL_1_SHDW  0x50000800
#define DDR3_EMIF_DDR_PHY_CTRL_2       0x07

/* Relative Register Definitions Using SOC_EMIF_0_REGS */
#define DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0200)
#define DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0204)
#define DATA0_REG_PHY_WRLVL_INIT_RATIO_0   (SOC_EMIF_0_REGS + 0x0208)
#define DATA0_REG_PHY_GATELVL_INIT_RATIO_0 (SOC_EMIF_0_REGS + 0x020C)
#define DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0210)
#define DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0214)
#define DATA0_REG_PHY_DLL_LOCK_DIFF_0      (SOC_EMIF_0_REGS + 0x0218)
#define DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0220)
#define DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0224)
#define DATA1_REG_PHY_WRLVL_INIT_RATIO_0   (SOC_EMIF_0_REGS + 0x0228)
#define DATA1_REG_PHY_GATELVL_INIT_RATIO_0 (SOC_EMIF_0_REGS + 0x022C)
#define DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0230)
#define DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0 (SOC_EMIF_0_REGS + 0x0234)
#define DATA1_REG_PHY_DLL_LOCK_DIFF_0      (SOC_EMIF_0_REGS + 0x0238)
#define CMD0_REG_PHY_CTRL_SLAVE_RATIO_0    (SOC_EMIF_0_REGS + 0x0100)
#define CMD0_REG_PHY_DLL_LOCK_DIFF_0       (SOC_EMIF_0_REGS + 0x0104)
#define CMD0_REG_PHY_INVERT_CLKOUT_0       (SOC_EMIF_0_REGS + 0x0108)
#define CMD1_REG_PHY_CTRL_SLAVE_RATIO_0    (SOC_EMIF_0_REGS + 0x0110)
#define CMD1_REG_PHY_DLL_LOCK_DIFF_0       (SOC_EMIF_0_REGS + 0x0114)
#define CMD1_REG_PHY_INVERT_CLKOUT_0       (SOC_EMIF_0_REGS + 0x0118)
#define CMD2_REG_PHY_CTRL_SLAVE_RATIO_0    (SOC_EMIF_0_REGS + 0x0120)
#define CMD2_REG_PHY_DLL_LOCK_DIFF_0       (SOC_EMIF_0_REGS + 0x0124)
#define CMD2_REG_PHY_INVERT_CLKOUT_0       (SOC_EMIF_0_REGS + 0x0128)


#define CM_DIV_M2_DPLL_MPU 0x44E0059C

void init_ddr3() {
    volatile int i;
    uart_puts("Initializing DDR3...\n");

    /* Call the new helper functions for DDR PHY and IO configuration */
    configure_ddr_phy();
    configure_ddr_phy_cmd_data();
    configure_ddr_io();
    configure_emif_zq();


    uart_puts("Initializing SDRAM timing\n");

    /* SDRAM Timing registers */
    *(volatile uint32_t *)EMIF_SDRAM_TIM_1 = 0x199DFB1B;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_1_SHDW = 0x199DFB1B;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_2 = 0x269E7FDA;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_2_SHDW = 0x269E7FDA;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_3 = 0x501F895F;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_3_SHDW = 0x501F895F;


    uart_puts("Initializing SDRAM ref controls\n");

    /* SDRAM Refresh Control registers */
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL = 0x00000C30;
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL_SHDW = 0x00000C30;

    /* SDRAM Configuration register */
    *(volatile uint32_t *)EMIF_SDRAM_CONFIG = 0x60A452B2;

    /* Disable EMIF power management */
    *(volatile uint32_t *)EMIF_PWR_MGMT_CTRL = 0x00000000;

    uart_puts("DDR3 Initialization Complete!\n");
}

void enable_emif_clocks(void) {
    volatile uint32_t timeout = TIMEOUT;
    
    uart_puts("Enabling EMIF clocks...\n");
    
    /* Enable Clocks: */
    /* Set the L3 clock domain's transition control (CLKTRCTRL) to 2 */
    *(volatile uint32_t *)CM_PER_L3_CLKSTCTRL = 2;
    
    /* Set the L3 clock control module mode to 2 (active) */
    *(volatile uint32_t *)CM_PER_L3_CLKCTRL = 2;
    
    /* Enable the functional clock for EMIF by setting its module mode to 2 */
    *(volatile uint32_t *)CM_PER_EMIF_CLKCTRL = 2;
   

    uart_puts("Waiting EMIF Clock\n");
    

    /* Wait until the EMIF clock becomes active (i.e. the IDLEST bit clears) */
    while ((*(volatile uint32_t *)CM_PER_EMIF_CLKCTRL & EMIF_CLKCTRL_IDLEST_MASK) && timeout--) {
        /* Optionally insert a small delay here if needed */
	if((timeout % 1000) == 0){
    	    uart_puts("Waiting EMIF Enable\n");
	}
    }
    
    if (timeout == 0) {
        uart_puts("ERROR: EMIF clock did not become active!\n");
        return;
    }
    
    uart_puts("EMIF clocks enabled successfully.\n");
}


void configure_ddr_phy() {
    uart_puts("Configuring DDR PHY...\n");

    uint32_t timeout = TIMEOUT;
    /* Wait until L3 Clock domain is active */
    while (((*(volatile uint32_t *)(CM_PER_BASE + CM_PER_L3_CLKSTCTRL) & 0x1F) != 0x1F) && timeout--) {}
    if (timeout == 0) {
        uart_puts("ERROR: L3 Clock inactive! DDR PHY cannot configure VTP.\n");
        return;
    }

    /* Enable VTP and perform CLRZ sequence for calibration */
    volatile uint32_t *vtp_ctrl = (volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_VTP_CTRL);
    *vtp_ctrl |= CONTROL_VTP_CTRL_ENABLE;
    *vtp_ctrl &= ~CONTROL_VTP_CTRL_CLRZ;   /* Clear CLRZ */
    *vtp_ctrl |= CONTROL_VTP_CTRL_CLRZ;    /* Set CLRZ */

    timeout = TIMEOUT;
    while (!(*vtp_ctrl & CONTROL_VTP_CTRL_READY) && timeout--) {}
    if (timeout == 0) {
        uart_puts("ERROR: VTP calibration timed out!\n");
    } else {
        uart_puts("VTP calibrated successfully.\n");
    }
}

void configure_ddr_phy_cmd_data() {
    uart_puts("Configuring DDR PHY CMD and Data registers...\n");
    
    /* DDR PHY CMD0 configuration */
    *(volatile uint32_t *)CMD0_REG_PHY_CTRL_SLAVE_RATIO_0 = 0x80;
    *(volatile uint32_t *)CMD0_REG_PHY_DLL_LOCK_DIFF_0    = 0x1;
    *(volatile uint32_t *)CMD0_REG_PHY_INVERT_CLKOUT_0      = 0x0;
    
    /* DDR PHY CMD1 configuration */
    *(volatile uint32_t *)CMD1_REG_PHY_CTRL_SLAVE_RATIO_0 = 0x80;
    *(volatile uint32_t *)CMD1_REG_PHY_DLL_LOCK_DIFF_0    = 0x1;
    *(volatile uint32_t *)CMD1_REG_PHY_INVERT_CLKOUT_0      = 0x0;
    
    /* DDR PHY CMD2 configuration */
    *(volatile uint32_t *)CMD2_REG_PHY_CTRL_SLAVE_RATIO_0 = 0x80;
    *(volatile uint32_t *)CMD2_REG_PHY_DLL_LOCK_DIFF_0    = 0x1;
    *(volatile uint32_t *)CMD2_REG_PHY_INVERT_CLKOUT_0      = 0x0;
    
    /* DDR PHY Data Macro 0 configuration */
    *(volatile uint32_t *)DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0   = 0x40;
    *(volatile uint32_t *)DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0   = 0x00;
    *(volatile uint32_t *)DATA0_REG_PHY_WRLVL_INIT_RATIO_0     = 0x0;
    *(volatile uint32_t *)DATA0_REG_PHY_GATELVL_INIT_RATIO_0     = 0x0;
    *(volatile uint32_t *)DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0    = 0x7B;
    *(volatile uint32_t *)DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0    = 0x80;
    *(volatile uint32_t *)DATA0_REG_PHY_DLL_LOCK_DIFF_0        = 1;
    
    /* DDR PHY Data Macro 1 configuration */
    *(volatile uint32_t *)DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0   = 0x40;
    *(volatile uint32_t *)DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0   = 0x00;
    *(volatile uint32_t *)DATA1_REG_PHY_WRLVL_INIT_RATIO_0     = 0x0;
    *(volatile uint32_t *)DATA1_REG_PHY_GATELVL_INIT_RATIO_0     = 0x0;
    *(volatile uint32_t *)DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0    = 0x7B;
    *(volatile uint32_t *)DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0    = 0x80;
    *(volatile uint32_t *)DATA1_REG_PHY_DLL_LOCK_DIFF_0        = 1;
    
    uart_puts("DDR PHY CMD and Data registers configured.\n");
}


void configure_ddr_io() {
    uart_puts("Configuring DDR IO...\n");

    /* Set DDR CMD IO configuration to 0x16B (for channels 0, 1, 2) */
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CMD_IOCTRL(0)) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CMD_IOCTRL(1)) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CMD_IOCTRL(2)) = 0x16B;

    /* Set DDR DATA IO configuration to 0x16B (for channels 0, 1) */
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_DATA_IOCTRL(0)) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_DATA_IOCTRL(1)) = 0x16B;

    /* Clear DDR IO Control bits for DDR3_RST_DEF_VAL, DDR_WUCLK_DISABLE, and MDDR_SEL */
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_IO_CTRL) &= ~(0x7);

    /* Enable DDR CKE Control */
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CKE_CTRL) |= (1 << 0); /* Assuming bit0 enables CKE */

    uart_puts("DDR IO Configuration Complete!\n");
}

void configure_emif_zq() {
    uart_puts("Configuring EMIF ZQ and Secure EMIF...\n");

    /* Write the reference values to the EMIF ZQ and SDRAM config registers */
    *(volatile uint32_t *)(SOC_EMIF_0_REGS + EMIF_ZQ_CONFIG) = 0x50074BE4;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_SECURE_EMIF_SDRAM_CONFIG) = 0x60A452B2;
    
    uart_puts("EMIF ZQ Configuration Complete!\n");
}

void enable_ddr_pll() {
    volatile uint32_t timeout;
    uart_puts("Initializing DDR PLL...\n");

    /* 1. Enter bypass mode: write DPLL_EN = 4 */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR = 0x4;

    /* 2. Wait for PLL to enter bypass mode: */
    /* In bypass, wait until: */
    /*    ST_DPLL_CLK (bit0) becomes 0  AND  ST_MN_BYPASS (bit1) becomes 1. */
    timeout = TIMEOUT;
    while (((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR & 0x1) != 0) || 
           (((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR) & 0x2) == 0)) {
        if (!(timeout--))
            break;
    }

    if (timeout == 0) {
        uart_puts("ERROR: DDR PLL did not enter bypass mode!\n");
        return;
    }

    /* 3. Disable Spread Spectrum Clocking (SSC) */
    /* Clear SSC enable bit (bit6, i.e. 0x40) */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR &= ~0x40;

    /* 4. Set DDR PLL multiplier and divider: Multiplier = 400, Divider = 23 (i.e. 24-1) */
    *(volatile uint32_t *)CM_CLKSEL_DPLL_DDR = (400 << 8) | (24 - 1);

    /* 5. Set DDR PLL post-divider (M2) to 1 */
    *(volatile uint32_t *)CM_DIV_M2_DPLL_MPU = 1;

    /* 6. Exit bypass: set DPLL_EN = 7 (Lock mode) */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR = 0x7;

    /* 7. Wait for PLL to lock: now wait until: */
    /*    ST_DPLL_CLK (bit0) becomes 1  AND  ST_MN_BYPASS (bit1) becomes 0. */
    timeout = TIMEOUT;
    while (((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR & 0x1) == 0) || 
           (((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR) & 0x2) != 0)) {
        if (!(timeout--))
            break;
    }

    if (timeout == 0) {
        uart_puts("ERROR: DDR PLL failed to lock!\n");
        return;
    }

    uart_puts("DDR PLL Initialized Successfully.\n");
}

void enable_core_pll() {
    volatile uint32_t timeout;
    uart_puts("Initializing Core PLL...\n");

    /* 1. Enter bypass mode: set DPLL_EN = 4 */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_CORE = 0x4;

    /* 2. Wait for PLL to enter bypass mode: */
    /* Wait until: */
    /*    ST_DPLL_CLK (bit0) becomes 0  AND  ST_MN_BYPASS (bit1) becomes 1. */
    timeout = TIMEOUT;
    while (((*(volatile uint32_t *)CM_IDLEST_DPLL_CORE & 0x1) != 0) || 
           (((*(volatile uint32_t *)CM_IDLEST_DPLL_CORE) & 0x2) == 0)) {
        if (!(timeout--))
            break;
    }

    if (timeout == 0) {
        uart_puts("ERROR: Core PLL did not enter bypass mode!\n");
        return;
    }

    /* 3. Disable Spread Spectrum Clocking (SSC) */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_CORE &= ~0x40;

    /* 4. Set Core PLL multiplier and divider: Multiplier = 1000, Divider = 23 (i.e. 24-1) */
    *(volatile uint32_t *)CM_CLKSEL_DPLL_CORE = (1000 << 8) | (24 - 1);

    /* 5. Set Core PLL post-dividers: */
    *(volatile uint32_t *)CM_DIV_M4_DPLL_CORE = 10;  /* Example: M4 divider = 10 */
    *(volatile uint32_t *)CM_DIV_M5_DPLL_CORE = 8;   /* Example: M5 divider = 8 */
    *(volatile uint32_t *)CM_DIV_M6_DPLL_CORE = 4;   /* Example: M6 divider = 4 */

    /* 6. Exit bypass: set DPLL_EN = 7 (Lock mode) */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_CORE = 0x7;

    /* 7. Wait for PLL to lock: now wait until: */
    /*    ST_DPLL_CLK (bit0) becomes 1  AND  ST_MN_BYPASS (bit1) becomes 0. */
    timeout = TIMEOUT;
    while (((*(volatile uint32_t *)CM_IDLEST_DPLL_CORE & 0x1) == 0) || 
           (((*(volatile uint32_t *)CM_IDLEST_DPLL_CORE) & 0x2) != 0)) {
        if (!(timeout--))
            break;
    }

    if (timeout == 0) {
        uart_puts("ERROR: Core PLL failed to lock!\n");
        return;
    }

    uart_puts("Core PLL Initialized Successfully.\n");
}




void enable_ddr_clock() {
    uart_puts("Enabling DDR Clock...\n");

    /* Wait until DDR PLL is locked */
    uint32_t timeout = TIMEOUT;
    while ((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR & 0x1) && timeout--) {}

    if (timeout == 0) {
        uart_puts("ERROR: DDR PLL Failed to Lock!\n");
        return;
    } else {
        uart_puts("DDR PLL Locked Successfully.\n");
    }

    /* **CRITICAL FIX HERE**: properly set the DDR clock divider (M2 divider) */
    *(volatile uint32_t *)CM_DIV_M2_DPLL_DDR = (1 | (1 << 8));  /* Divider = 1, Enable M2 divider bit (8) */

    /* Re-lock DDR PLL to apply divider */
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR = 0x7;  /* Lock mode again */


    /* Enable EMIF clock explicitly */
    *(volatile uint32_t *)(CM_PER_BASE + CM_PER_EMIF_CLKCTRL) |= 0x2;
    timeout = TIMEOUT;
    /*while (((*(volatile uint32_t *)(CM_PER_BASE + CM_PER_EMIF_CLKCTRL)) & 0x3) != 0x2 && timeout--) {} */

    uart_puts("DDR Clock Enabled Successfully.\n");
}




void setup_memory() {
    enable_core_pll();
    enable_ddr_pll();
    enable_emif_clocks();
    /*enable_ddr_clock(); */
    init_ddr3();
}

int test_ddr3_memory() {
    uint32_t *mem_addr;
    uint32_t patterns[] = {0xAAAAAAAA, 0x55555555, 0x00000000, 0xFFFFFFFF};
    int errors = 0;
    int p;



    uart_puts("Checking PLL Lock Status...\n");
    uint32_t pll_status = *(volatile uint32_t *)CM_IDLEST_DPLL_DDR;
    uart_print_hex(pll_status);
    uart_puts("\n");


    uart_puts("Checking DDR Clock...\n");
    uint32_t ddr_clock = *(volatile uint32_t *)CM_DIV_M2_DPLL_DDR;
    uart_print_hex(ddr_clock);
    uart_puts("\n");

    uart_puts("Checking EMIF SDRAM Status...\n"); 
    uint32_t emif_sdram_status = *(volatile uint32_t *)EMIF_SDRAM_STATUS;
    uart_puts("EMIF Status: ");
    uart_print_hex(emif_sdram_status);
    uart_puts("\n");


    uart_puts("Checking DDR PLL Status...\n");
    uint32_t ddr_pll_status = *(volatile uint32_t *)CM_IDLEST_DPLL_DDR;
    uart_print_hex(ddr_pll_status);
    uart_puts("\n");

    uart_puts("Checking DDR PLL Frequency Selection...\n");
    uint32_t ddr_pll_config = *(volatile uint32_t *)CM_CLKSEL_DPLL_DDR;
    uart_print_hex(ddr_pll_config);
    uart_puts("\n");


    uart_puts("Checking EMIF Power Management...\n");
    uint32_t emif_pwr_mgmt = *(volatile uint32_t *)EMIF_PWR_MGMT_CTRL;
    uart_puts("Power Management Register: ");
    uart_print_hex(emif_pwr_mgmt);
    uart_puts("\n");

    uart_puts("Checking EMIF Clock Status...\n");
    uint32_t emif_clk_status = *(volatile uint32_t *)CM_PER_EMIF_CLKCTRL;
    uart_print_hex(emif_clk_status);
    uart_puts("\n");

    uart_puts("Checking PRCM DDR Power Control...\n");
    uint32_t prcm_ddr_pwr = *(volatile uint32_t *)CM_PER_L3_CLKSTCTRL;
    uart_print_hex(prcm_ddr_pwr);
    uart_puts("\n");


    uart_puts("Checking DDR Clock Control Register...\n");
    uint32_t ddr_clk_ctrl = *(volatile uint32_t *)CM_CLKSEL_DPLL_DDR;
    uart_print_hex(ddr_clk_ctrl);
    uart_puts("\n");

    uart_puts("Checking DDR Clock Divider...\n");
    uint32_t ddr_clk_div = *(volatile uint32_t *)CM_DIV_M2_DPLL_DDR;
    uart_print_hex(ddr_clk_div);
    uart_puts("\n");


    uart_puts("DDR3 Memory Test Start\n");

    for (p = 0; p < 4; p++) {
        uint32_t test_pattern = patterns[p];


	for (mem_addr = (uint32_t *)DDR_START_ADDR;
			mem_addr < (uint32_t *)(DDR_START_ADDR + DDR_SIZE);
             mem_addr++) {
            *mem_addr = test_pattern;
            uint32_t read_back = *mem_addr;

            if (read_back != test_pattern) {
                errors++; 
	    }
        }
	

    	uart_puts("DDR3 Memory Test Mid\n");

    }

    if (errors > 0) {
        uart_puts("Memory Test Failed! Errors: ");
        uart_print_hex(errors);
        uart_puts("\n");
    } else {
        uart_puts("Memory Test Passed!\n");
    }

    return errors;
}

void precharge_ddr3() {
    volatile int i;
    uart_puts("Ensuring DDR3 refresh is enabled before disabling...\n");

    /* Ensure DDR3 refresh is ON before disabling it */
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL &= ~(1 << 31);

    for (i = 0; i < 10000; i++);  /* Small delay for stabilization */

    uart_puts("DDR3 Refresh Ensured.\n");
}




void itoa(int num, char *str, int base) {
    int i = 0;
    int isNegative = 0;

    /* Handle 0 explicitly */
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    /* Handle negative numbers for base 10 */
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    /* Manually convert number to string (without / or %) */
    while (num > 0) {
        int quotient = 0, remainder = num;

        /* Manual division (bitwise shifting for powers of 2) */
        while (remainder >= base) {
            remainder -= base;
            quotient++;
        }

        str[i++] = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
        num = quotient;  /* Move to next digit */
    }

    /* Add '-' sign if negative */
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';

    /* Reverse the string */
    int start = 0, end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


void uart_print_hex(uint32_t value) {
    char buffer[12];  /* Enough space for "0x" + 8 hex digits + null terminator */
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = '\0';

    itoa(value, buffer + 2, 16);  /* Convert value to hex starting at buffer[2] */

    uart_puts(buffer);  /* Send hex string over UART */
    uart_puts("\n");
}
