#include <types.h> 
#include <ddr.h>
#include <uart.h>

#define TIMEOUT 1000000  // Prevent infinite loops

#define CM_CLKMODE_DPLL_DDR   0x44E00594  // DPLL DDR Clock Mode Register
#define CM_IDLEST_DPLL_DDR    0x44E00520  // DPLL DDR Status Register
#define CM_CLKSEL_DPLL_DDR    0x44E00598  // DPLL DDR Multiplier/Divider Register
#define CM_DIV_M2_DPLL_DDR    0x44E0059C  // DDR DPLL M2 Divider Register
#define CM_PER_L3_CLKSTCTRL      0x44E00000  // L3 Clock Standby Control
#define VTP_CTRL   0x44E10E0C  // VTP Control Register


#define CM_CLKMODE_DPLL_CORE    0x44E00490  // Core PLL Mode
#define CM_CLKSEL_DPLL_CORE     0x44E00468  // Core PLL Multiplier/Divider
#define CM_IDLEST_DPLL_CORE     0x44E0045C  // Core PLL Status


#define EMIF_SDRAM_CONFIG        0x4C000008  // SDRAM Configuration Register
#define EMIF_SDRAM_CONFIG_2      0x4C00000C  // SDRAM Configuration Register 2
#define EMIF_SDRAM_REF_CTRL      0x4C000010  // SDRAM Refresh Control Register
#define EMIF_SDRAM_REF_CTRL_SHDW 0x4C000014  // SDRAM Refresh Control Shadow Register
#define EMIF_SDRAM_TIM_1         0x4C000018  // SDRAM Timing Register 1
#define EMIF_SDRAM_TIM_1_SHDW    0x4C00001C  // SDRAM Timing Register 1 Shadow
#define EMIF_SDRAM_TIM_2         0x4C000020  // SDRAM Timing Register 2
#define EMIF_SDRAM_TIM_2_SHDW    0x4C000024  // SDRAM Timing Register 2 Shadow
#define EMIF_SDRAM_TIM_3         0x4C000028  // SDRAM Timing Register 3
#define EMIF_SDRAM_TIM_3_SHDW    0x4C00002C  // SDRAM Timing Register 3 Shadow
#define EMIF_PWR_MGMT_CTRL       0x4C000038  // Power Management Control Register
#define EMIF_PWR_MGMT_CTRL_SHDW  0x4C00003C  // Power Management Control Shadow Register

#define EMIF_DDR_PHY_CTRL_1           0x4C0000E4  // DDR PHY Control Register 1
#define EMIF_DDR_PHY_CTRL_1_SHDW      0x4C0000E8  // DDR PHY Control Register 1 Shadow

#define EMIF_SDRAM_STATUS  0x4C00004C  // Found in EMIF registers
#define EMIF_PWR_MGMT_CTRL  0x4C000038


// Base addresses
#define CM_PER_BASE                        0x44E00000  // Clock Management Peripheral Base
#define CONTROL_MODULE_BASE                0x44E10000  // Control Module Base
#define SOC_EMIF_0_REGS                    0x4C000000  // EMIF Base Address

// EMIF Clock Control Registers
#define CM_PER_EMIF_CLKCTRL                0x44E00028  // EMIF Clock Control
#define CM_PER_EMIF_FW_CLKCTRL             0x44E0002C  // EMIF Functional Clock Control
#define CM_PER_EMIF_CLKCTRL_MODULEMODE_ENABLE  0x2
#define CM_PER_EMIF_FW_CLKCTRL_MODULEMODE_ENABLE  0x2

// EMIF Power and Clock Status
#define CM_PER_L3_CLKSTCTRL                0x44E00004  // L3 Clock Standby Control
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK  0x00000002
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK    0x00000001

// DDR PHY Control Registers
#define CONTROL_VTP_CTRL                   0x44E10E0C  // VTP Control Register
#define CONTROL_VTP_CTRL_ENABLE            0x00000040  // Enable VTP
#define CONTROL_VTP_CTRL_CLRZ              0x00000020  // Clear VTP
#define CONTROL_VTP_CTRL_READY             0x00000080  // VTP Ready Bit

// DDR CMD IO Control Registers
#define CONTROL_DDR_CMD_IOCTRL(n)          (0x44E10E30 + ((n) * 4)) // DDR CMD IOCTRL 0-2
#define CONTROL_DDR_DATA_IOCTRL(n)         (0x44E10E60 + ((n) * 4)) // DDR Data IOCTRL 0-1
#define CONTROL_DDR_IO_CTRL                0x44E10E90  // DDR IO Control Register
#define CONTROL_DDR_CKE_CTRL               0x44E10E94  // DDR CKE Control Register
#define CONTROL_DDR_CKE_CTRL_DDR_CKE_CTRL  0x00000001

// EMIF Registers
#define EMIF_SDRAM_CONFIG                  0x4C000008  // SDRAM Configuration
#define EMIF_DDR_PHY_CTRL_1                0x4C0000E4  // DDR PHY Control Register 1
#define EMIF_DDR_PHY_CTRL_1_SHDW           0x4C0000E8  // DDR PHY Control 1 Shadow
#define EMIF_DDR_PHY_CTRL_2                0x4C0000EC  // DDR PHY Control Register 2
#define EMIF_SDRAM_REF_CTRL                0x4C000010  // SDRAM Refresh Control
#define EMIF_SDRAM_REF_CTRL_SHDW           0x4C000014  // SDRAM Refresh Control Shadow
#define EMIF_ZQ_CONFIG                     0x4C0000C8  // ZQ Configuration
#define CONTROL_SECURE_EMIF_SDRAM_CONFIG   0x44E10E0C  // Secure EMIF Config

// DDR PHY Timing and Configuration Registers
#define CMD0_SLAVE_RATIO_0                 0x44E12000
#define CMD1_SLAVE_RATIO_0                 0x44E12004
#define CMD2_SLAVE_RATIO_0                 0x44E12008
#define CMD0_INVERT_CLKOUT_0               0x44E1200C
#define CMD1_INVERT_CLKOUT_0               0x44E12010
#define CMD2_INVERT_CLKOUT_0               0x44E12014

// Data Slave Ratio Registers (for DDR tuning)
#define DATA0_RD_DQS_SLAVE_RATIO_0         0x44E12018
#define DATA0_WR_DQS_SLAVE_RATIO_0         0x44E1201C
#define DATA0_FIFO_WE_SLAVE_RATIO_0        0x44E12020
#define DATA0_WR_DATA_SLAVE_RATIO_0        0x44E12024
#define DATA1_RD_DQS_SLAVE_RATIO_0         0x44E12028
#define DATA1_WR_DQS_SLAVE_RATIO_0         0x44E1202C
#define DATA1_FIFO_WE_SLAVE_RATIO_0        0x44E12030
#define DATA1_WR_DATA_SLAVE_RATIO_0        0x44E12034

// DDR3-Specific Values (Ensure these are correctly defined)
#define DDR3_CMD0_SLAVE_RATIO_0            0x80  // Example value, adjust per board
#define DDR3_CMD1_SLAVE_RATIO_0            0x80
#define DDR3_CMD2_SLAVE_RATIO_0            0x80
#define DDR3_CMD0_INVERT_CLKOUT_0          0x00
#define DDR3_CMD1_INVERT_CLKOUT_0          0x00
#define DDR3_CMD2_INVERT_CLKOUT_0          0x00
#define DDR3_DATA0_RD_DQS_SLAVE_RATIO_0    0x38
#define DDR3_DATA0_WR_DQS_SLAVE_RATIO_0    0x00
#define DDR3_DATA0_FIFO_WE_SLAVE_RATIO_0   0x80
#define DDR3_DATA0_WR_DATA_SLAVE_RATIO_0   0x80
#define DDR3_DATA0_RD_DQS_SLAVE_RATIO_1    0x38
#define DDR3_DATA0_WR_DQS_SLAVE_RATIO_1    0x00
#define DDR3_DATA0_FIFO_WE_SLAVE_RATIO_1   0x80
#define DDR3_DATA0_WR_DATA_SLAVE_RATIO_1   0x80
#define DDR3_CONTROL_DDR_CMD_IOCTRL_0      0x18B
#define DDR3_CONTROL_DDR_CMD_IOCTRL_1      0x18B
#define DDR3_CONTROL_DDR_CMD_IOCTRL_2      0x18B
#define DDR3_CONTROL_DDR_DATA_IOCTRL_0     0x18B
#define DDR3_CONTROL_DDR_DATA_IOCTRL_1     0x18B
#define DDR3_CONTROL_DDR_IO_CTRL           0x18B
#define DDR3_CONTROL_DDR_CKE_CTRL          0x00000001
#define DDR3_EMIF_ZQ_CONFIG_VAL            0x50074BE4
#define DDR3_EMIF_SDRAM_CONFIG             0x61851B32
#define DDR3_EMIF_SDRAM_REF_CTRL_VAL1      0x00001035
#define DDR3_EMIF_SDRAM_REF_CTRL_SHDW_VAL1 0x00001035
#define DDR3_EMIF_DDR_PHY_CTRL_1           0x50000800
#define DDR3_EMIF_DDR_PHY_CTRL_1_SHDW      0x50000800
#define DDR3_EMIF_DDR_PHY_CTRL_2           0x07


#define CM_PER_EMIF_FW_CLKCTRL_MODULEMODE           (0x3)  // Mode bits mask
#define CM_PER_EMIF_FW_CLKCTRL_MODULEMODE_ENABLE    (0x2)  // Enable mode value

#define CM_PER_EMIF_CLKCTRL_MODULEMODE           (0x3)  // Mode bits mask
#define CM_PER_EMIF_CLKCTRL_MODULEMODE_ENABLE    (0x2)  // Enable mode value

#define PRCM_L3_PWR_CTRL        0x44E00000  // Power control for L3 (Example address, verify with TRM)
#define PRCM_EMIF_PWR_CTRL      0x44E00004  // Power control for EMIF (Example address, verify with TRM)

#define CM_PER_L3_CLKSTCTRL     0x44E0000C  // L3 Clock Standby Control
#define CM_PER_EMIF_CLKCTRL     0x44E00028  // EMIF Functional Clock Control
#define CM_PER_EMIF_FW_CLKCTRL  0x44E0002C  // EMIF FW Clock Control

#define CM_IDLEST_DPLL_DDR      0x44E00520  // DDR PLL Idle Status
#define CM_DIV_M2_DPLL_DDR      0x44E0059C  // DDR Clock Divider

#define CONTROL_MODULE_BASE     0x44E10000  // Control Module Base Address
#define CONTROL_VTP_CTRL        0x44E10E0C  // VTP Control Register
#define CONTROL_DDR_IO_CTRL     0x44E10E10  // DDR IO Control Register


void init_ddr3() {
    volatile int i;
    uart_puts("Initializing DDR3...\n");

    configure_ddr_phy();
    configure_ddr_io();
    configure_emif_zq();

    // DDR PHY Control
    *(volatile uint32_t *)EMIF_DDR_PHY_CTRL_1 = 0x849FFFF5;
    *(volatile uint32_t *)EMIF_DDR_PHY_CTRL_1_SHDW = 0x849FFFF5;

    // DDR Timings
    *(volatile uint32_t *)EMIF_SDRAM_TIM_1 = 0x0AAAD4DB;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_2 = 0x26437FDA;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_3 = 0x501F84EF;

    // SDRAM Configuration
    *(volatile uint32_t *)EMIF_SDRAM_CONFIG = 0x61851B32;

    // Refresh Rates
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL = 0x80001035;  // Initial Refresh
    for (i = 0; i < 1000; i++); // Short wait
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL = 0x00001035;

    // Disable power management
    *(volatile uint32_t *)EMIF_PWR_MGMT_CTRL = 0x00000000;

    uart_puts("DDR3 Initialization Complete!\n");
}


void enable_emif_clocks() {
    volatile uint32_t timeout;
    volatile int delay;

    uart_puts("Enabling EMIF clocks...\n");

    // STEP 1: Ensure L3 and EMIF domains powered up
    uart_puts("Powering up L3 domain...\n");
    *(volatile uint32_t *)PRCM_L3_PWR_CTRL = 0x3;
    for(delay=0; delay<10000; delay++);

    uart_puts("Powering up EMIF domain...\n");
    *(volatile uint32_t *)PRCM_EMIF_PWR_CTRL = 0x3;
    for(delay=0; delay<10000; delay++);

    // STEP 2: Verify Core PLL locked again explicitly
    uart_puts("Verifying Core PLL lock...\n");
    timeout = 100000;
    while ((*(volatile uint32_t *)CM_IDLEST_DPLL_CORE & 0x1) && timeout--) {}
    if (!timeout) uart_puts("ERROR: Core PLL lock timeout!\n");

    // STEP 3: Enable L3 Clock domain explicitly
    uart_puts("Enabling L3 Clock Domain...\n");
    *(volatile uint32_t *)(CM_PER_BASE + CM_PER_L3_CLKSTCTRL) = 0x2;

    timeout = 100000;
    while ((*(volatile uint32_t *)(CM_PER_BASE + CM_PER_L3_CLKSTCTRL) & 0x100) == 0 && timeout--) {
        if(timeout % 10000 == 0) uart_puts("Waiting for L3 Clock Domain...\n");
    }
    if (!timeout) {
        uart_puts("ERROR: L3 Clock domain activation failed!\n");
        return;
    }

    uart_puts("L3 Clock Domain activated successfully.\n");

    // STEP 4: Enable EMIF Module clock
    uart_puts("Enabling EMIF Module Clock...\n");
    *(volatile uint32_t *)(CM_PER_BASE + CM_PER_EMIF_CLKCTRL) = 0x2;

    timeout = 100000;
    while ((*(volatile uint32_t *)(CM_PER_BASE + CM_PER_EMIF_CLKCTRL) & 0x30000) != 0 && timeout--) {
        if(timeout % 10000 == 0) uart_puts("Waiting for EMIF module clock...\n");
    }
    if (!timeout) {
        uart_puts("ERROR: EMIF Module clock enable timeout!\n");
        return;
    }

    uart_puts("EMIF clocks enabled successfully.\n");
}




void configure_ddr_phy() {
    uart_puts("Configuring DDR PHY...\n");

    // Ensure L3 Clock is active before proceeding
    uint32_t timeout = TIMEOUT;
    while (((*(volatile uint32_t *)(CM_PER_BASE + CM_PER_L3_CLKSTCTRL) & 0x1F) != 0x1F) && timeout--) {}
    if (timeout == 0) {
        uart_puts("ERROR: L3 Clock inactive! DDR PHY cannot configure VTP.\n");
        return;
    }

    // Enable VTP and wait for calibration
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_VTP_CTRL) |= CONTROL_VTP_CTRL_ENABLE;
    timeout = TIMEOUT;
    while (!(*(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_VTP_CTRL) & CONTROL_VTP_CTRL_READY) && timeout--) {}
    if (timeout == 0) uart_puts("ERROR: VTP calibration timed out!\n");
}




void configure_ddr_io() {
    uart_puts("Configuring DDR IO...\n");

    // DDR CMD IO Configuration
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CMD_IOCTRL(0)) = DDR3_CONTROL_DDR_CMD_IOCTRL_0;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CMD_IOCTRL(1)) = DDR3_CONTROL_DDR_CMD_IOCTRL_1;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CMD_IOCTRL(2)) = DDR3_CONTROL_DDR_CMD_IOCTRL_2;
    
    // DDR DATA IO Configuration
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_DATA_IOCTRL(0)) = DDR3_CONTROL_DDR_DATA_IOCTRL_0;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_DATA_IOCTRL(1)) = DDR3_CONTROL_DDR_DATA_IOCTRL_1;

    // DDR IO Control
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_IO_CTRL) &= DDR3_CONTROL_DDR_IO_CTRL;

    // Enable DDR CKE Control
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_DDR_CKE_CTRL) |= CONTROL_DDR_CKE_CTRL_DDR_CKE_CTRL;

    uart_puts("DDR IO Configuration Complete!\n");
}

void configure_emif_zq() {
    uart_puts("Configuring EMIF ZQ and Secure EMIF...\n");

    *(volatile uint32_t *)(SOC_EMIF_0_REGS + EMIF_ZQ_CONFIG) = DDR3_EMIF_ZQ_CONFIG_VAL;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + CONTROL_SECURE_EMIF_SDRAM_CONFIG) = DDR3_EMIF_SDRAM_CONFIG;

    uart_puts("EMIF ZQ Configuration Complete!\n");
}


void enable_ddr_pll() {
    uart_puts("Enabling DDR PLL...\n");

    // Step 1: Set DDR PLL to bypass mode
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR = 0x4;  // Bypass mode

    // Step 2: Set DDR PLL Multiplier & Divider
    *(volatile uint32_t *)CM_CLKSEL_DPLL_DDR = (23 << 8) | 1;
    // Example values: Mult = 23, Div = 1 (adjust based on platform requirements)

    // Step 3: Enable DDR PLL (Lock Mode)
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR = 0x7;  // Lock mode

    // Step 4: Wait for PLL to lock
    uint32_t timeout = TIMEOUT;
    while ((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR & 0x1) && timeout--) {}

    if (timeout == 0) {
        uart_puts("ERROR: DDR PLL failed to lock!\n");
        return;
    }
    uart_puts("DDR PLL Enabled Successfully.\n");
}

void enable_core_pll() {
    volatile uint32_t prcm_core_status;
    volatile uint32_t pll_core_status;
    volatile uint32_t timeout;

    uart_puts("Enabling Core PLL...\n");

    // STEP 1: Explicitly FORCE Core Power ON and VERIFY
    uart_puts("Checking PRCM Core Power Control...\n");
    prcm_core_status = *(volatile uint32_t *)PRCM_L3_PWR_CTRL;
    uart_print_hex(prcm_core_status);
    uart_puts("\n");

    if ((prcm_core_status & 0x3) != 0x3) {
        uart_puts("Core Power is not fully enabled. Fixing...\n");
        *(volatile uint32_t *)PRCM_L3_PWR_CTRL = 0x3; // Force ON explicitly

        // Verify and WAIT until power domain is FULLY ON
        uint32_t timeout = 100000;
        while(((*(volatile uint32_t *)PRCM_L3_PWR_CTRL) & 0x3) != 0x3 && timeout--) {
            if(timeout % 10000 == 0) uart_puts("Waiting for Core Power ON...\n");
        }
        if (timeout == 0) {
            uart_puts("ERROR: Core Power domain activation failed!\n");
            return;
        }
        uart_puts("Core Power domain activated successfully.\n");
    }

    // Small delay after power stabilization
    volatile int delay;
    for(delay=0; delay<10000; delay++);

    // STEP 2: Set Core PLL in BYPASS mode first
    uart_puts("Setting Core PLL to Bypass Mode...\n");
    *(volatile uint32_t *)CM_CLKMODE_DPLL_CORE = 0x4; // Bypass mode
    for(delay=0; delay<10000; delay++);

    // STEP 3: Configure Multiplier and Divider explicitly
    uart_puts("Configuring Core PLL Multiplier and Divider...\n");
    *(volatile uint32_t *)CM_CLKSEL_DPLL_CORE = (100 << 8) | 1;  // Multiplier=100, Divider=1

    // STEP 3: Lock Core PLL explicitly
    uart_puts("Locking Core PLL...\n");
    *(volatile uint32_t *)CM_CLKMODE_DPLL_CORE = 0x7;  // Lock mode

    timeout = 100000;
    while((*(volatile uint32_t *)CM_IDLEST_DPLL_CORE & 0x1) && timeout--) {
        if(timeout % 10000 == 0) uart_puts("Waiting for Core PLL lock...\n");
    }

    pll_core_status = *(volatile uint32_t *)CM_IDLEST_DPLL_CORE;
    uart_puts("Final Core PLL Status: ");
    uart_print_hex(pll_core_status);
    uart_puts("\n");

    if(pll_core_status & 0x1) {
        uart_puts("ERROR: Core PLL did NOT lock!\n");
        return;
    }

    uart_puts("Core PLL Enabled Successfully.\n");
}




void enable_ddr_clock() {
    uart_puts("Enabling DDR Clock...\n");

    // Wait until DDR PLL is locked
    uint32_t timeout = TIMEOUT;
    while ((*(volatile uint32_t *)CM_IDLEST_DPLL_DDR & 0x1) && timeout--) {}

    if (timeout == 0) {
        uart_puts("ERROR: DDR PLL Failed to Lock!\n");
        return;
    } else {
        uart_puts("DDR PLL Locked Successfully.\n");
    }

    // **CRITICAL FIX HERE**: properly set the DDR clock divider (M2 divider)
    *(volatile uint32_t *)CM_DIV_M2_DPLL_DDR = (1 | (1 << 8));  // Divider = 1, Enable M2 divider bit (8)

    // Re-lock DDR PLL to apply divider
    *(volatile uint32_t *)CM_CLKMODE_DPLL_DDR = 0x7;  // Lock mode again

    // Wait for M2 divider to be ready
    timeout = TIMEOUT;
    while (((*(volatile uint32_t *)CM_DIV_M2_DPLL_DDR) & (1 << 8)) == 0 && timeout--) {
        uart_puts("Waiting for DDR Clock Divider to Enable...\n");
    }

    if (timeout == 0) {
        uart_puts("ERROR: DDR Clock Divider failed to enable!\n");
        return;
    }

    // Enable EMIF clock explicitly
    *(volatile uint32_t *)(CM_PER_BASE + CM_PER_EMIF_CLKCTRL) |= 0x2;
    timeout = TIMEOUT;
    while (((*(volatile uint32_t *)(CM_PER_BASE + CM_PER_EMIF_CLKCTRL)) & 0x3) != 0x2 && timeout--) {}

    uart_puts("DDR Clock Enabled Successfully.\n");
}




void setup_memory() {
    enable_core_pll();
    enable_ddr_pll();
    enable_emif_clocks();
    enable_ddr_clock();
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

    // Ensure DDR3 refresh is ON before disabling it
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL &= ~(1 << 31);

    for (i = 0; i < 10000; i++);  // Small delay for stabilization

    uart_puts("DDR3 Refresh Ensured.\n");
}




void itoa(int num, char *str, int base) {
    int i = 0;
    int isNegative = 0;

    // Handle 0 explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    // Handle negative numbers for base 10
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }

    // Manually convert number to string (without / or %)
    while (num > 0) {
        int quotient = 0, remainder = num;

        // Manual division (bitwise shifting for powers of 2)
        while (remainder >= base) {
            remainder -= base;
            quotient++;
        }

        str[i++] = (remainder > 9) ? (remainder - 10) + 'A' : remainder + '0';
        num = quotient;  // Move to next digit
    }

    // Add '-' sign if negative
    if (isNegative)
        str[i++] = '-';

    str[i] = '\0';

    // Reverse the string
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
    char buffer[12];  // Enough space for "0x" + 8 hex digits + null terminator
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[2] = '\0';

    itoa(value, buffer + 2, 16);  // Convert value to hex starting at buffer[2]

    uart_puts(buffer);  // Send hex string over UART
    uart_puts("\n");
}
