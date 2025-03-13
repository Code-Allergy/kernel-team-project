#include <types.h> 
#include <ddr.h>
#include <uart.h>
#include <utils.h>

#define TIMEOUT 1000000  /* Prevent infinite loops */


/* Register definitions for AM335x (BeagleBone Black) in C90 style */
#define CONTROL_MODULE_BASE 		0x44E10000U
#define CM_PER_BASE         		0x44E00000U
#define CM_WKUP_BASE        		0x44E00400U
#define SOC_EMIF_0_REGS     		0x4C000000U
#define DDR_PHY_CTRL_BASE   		0x44E12000U

/* EMIF Clock Registers */
#define CM_PER_EMIF_CLKCTRL 		0x28
#define CM_PER_L3_CLKSTCTRL 		0x0C


/* DDR Control and Config Registers */
#define CM_DDR_CKE 		     	0x131C
#define CM_DDR_CKE_CTRL_DDR_CKE_CTRL 	0x00000001U
#define CM_CONTROL_EMIT_SDRAM_CONFIG 	0x110


/* DDR PLL Registers */
#define CM_IDLEST_DPLL_DDR 		0x34
#define CM_CLKMODE_DPLL_DDR 		0x94
#define DPLL_LOCK 			0x07


/* EMIF PHY Registers */
#define EMIF_MOD_ID_REV 		0x00U
#define STATUS 				0x04U
#define SDRAM_CONFIG 			0x08U
#define SDRAM_CONFIG_2 			0x0CU


/* DDR init specific registers */
#define EMIF_SDRAM_REF_CTRL 		0x4C000010U
#define EMIF_SDRAM_REF_CTRL_SHDW        0x4C000014U
#define EMIF_SDRAM_TIM_1 		0x4C000018U
#define EMIF_SDRAM_TIM_1_SHDW 		0x4C00001CU
#define EMIF_SDRAM_TIM_2 		0x4C000020U
#define EMIF_SDRAM_TIM_2_SHDW 		0x4C000024U
#define EMIF_SDRAM_TIM_3 		0x4C000028U
#define EMIF_SDRAM_TIM_3_SHDW 		0x4C00002CU



/* zq */
#define ZQ_CONFIG	 		0xC8U

/* Onchip Perif Config Registers */
#define OCP_CFG_VAL_1                   0x58U
#define OCP_CFG_VAL_2                   0x5CU


/* DDR3-Specific Values */
#define DDR3_CMD0_SLAVE_RATIO_0         0x80U
#define DDR3_CMD1_SLAVE_RATIO_0         0x80U
#define DDR3_CMD2_SLAVE_RATIO_0         0x80U
#define DDR3_CMD0_INVERT_CLKOUT_0       0x00U
#define DDR3_CMD1_INVERT_CLKOUT_0       0x00U
#define DDR3_CMD2_INVERT_CLKOUT_0       0x00U
#define DDR3_DATA0_RD_DQS_SLAVE_RATIO_0 0x38U

/* EMIF Power and Clock Status Flags */
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK 0x00000004U
#define CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK   0x00000010U

#define PRCM_L3_PWR_CTRL 0x44E00000U

#define CM_WKUP_CM_CLKSEL_DPLL_DDR_DPLL_MULT 0x0007FF00
#define CM_WKUP_CM_CLKSEL_DPLL_DDR_DPLL_DIV  0x0000007F




void init_ddr3() {

    log_message(LOG_LEVEL_DEBUG, "Initializing DDR...\n");
    
    configure_ddr_phy();
    configure_ddr_phy_cmd_data();
    configure_ddr_io();
 
    
    log_message(LOG_LEVEL_DEBUG, "PHY Config successfull.\n");

    *(volatile uint32_t *)EMIF_SDRAM_TIM_1 =         0x0AAAD4DB;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_1_SHDW =    0x0AAAD4DB;
   


 
    *(volatile uint32_t *)EMIF_SDRAM_TIM_2 =         0x266B7FDA;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_2_SHDW =    0x266B7FDA;
    
    *(volatile uint32_t *)EMIF_SDRAM_TIM_3 =         0x50074BE4;
    *(volatile uint32_t *)EMIF_SDRAM_TIM_3_SHDW =    0x50074BE4;
    
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL =      0x00000C30;
    *(volatile uint32_t *)EMIF_SDRAM_REF_CTRL_SHDW = 0x00000C30;


    log_message(LOG_LEVEL_DEBUG, "SDRAM Config successfull.\n");


    /* Export SDRAM config register to the EMIF */
    REG32_write(SOC_EMIF_0_REGS, SDRAM_CONFIG, 0x61C04BB2);
    REG32_write(CONTROL_MODULE_BASE, CM_CONTROL_EMIT_SDRAM_CONFIG, 0x61C00BB2);


    log_message(LOG_LEVEL_DEBUG, "DDR Initialization Complete.\n");
}


static inline void enable_emif_clocks(void)
{    

    /* L3 clock */
    *(volatile unsigned int *)(CM_PER_BASE + 0xe0)  |= 0x2U;

    /* L4LS */
    *(volatile unsigned int *)(CM_PER_BASE + 0x60)  |= 0x2U;

    /* L4FW */
    *(volatile unsigned int *)(CM_PER_BASE + 0x64)  |= 0x2U;

    /* L3 instr */
    *(volatile unsigned int *)(CM_PER_BASE + 0xdc)  |= 0x2U;

    /* L4HS */
    *(volatile unsigned int *)(CM_PER_BASE + 0x120) |= 0x2U;

    /* L4WKUP */
    *(volatile unsigned int *)(CM_WKUP_BASE + 0xc)  |= 0x2U;



    /* Wait for the clocks to stabalise */
    WAIT_FOR_REG32(CM_PER_BASE,  0xe0,  0x3U, 0x2U, TIMEOUT); 
    WAIT_FOR_REG32(CM_PER_BASE,  0x60,  0x3U, 0x2U, TIMEOUT);
    WAIT_FOR_REG32(CM_PER_BASE,  0x64,  0x3U, 0x2U, TIMEOUT);
    WAIT_FOR_REG32(CM_PER_BASE,  0xdc,  0x3U, 0x2U, TIMEOUT);
    WAIT_FOR_REG32(CM_PER_BASE,  0x120, 0x3U, 0x2U, TIMEOUT);
    WAIT_FOR_REG32(CM_WKUP_BASE, 0xc,   0x3U, 0x2U, TIMEOUT);
}

static inline void configure_ddr_phy(void)
{
    /* Enable VTP Control */
    *(volatile unsigned int *)(CONTROL_MODULE_BASE + 0xe0c) |= 0x40;

    /* Perform CLRZ sequence */
    *(volatile unsigned int *)(CONTROL_MODULE_BASE + 0xe0c) |= 0x01;

    /* Wait for VTP to activate */
    WAIT_FOR_REG32(CONTROL_MODULE_BASE, 0xe0c, 0x20, 0x20, TIMEOUT);
}

static inline void configure_ddr_phy_cmd_data(void) 
{
    /* DDR PHY CMD0 configuration */
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x1C)  = 0x80U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x2C)  = 0x00U;

    /* DDR PHY CMD1 configuration */
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x50)  = 0x80U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x60)  = 0x00U;

    /* DDR PHY CMD2 configuration */
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x84)  = 0x80U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x94)  = 0x00U; 

    /* DDR PHY Data Macro 0 configuration */
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0xC8)  = 0x38U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0xDC)  = 0x44U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x108) = 0x94U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x120) = 0x7DU;

    /* DDR PHY Data Macro 1 configuration */
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x170) = 0x38U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x184) = 0x44U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x1B0) = 0x94U;
    *(volatile unsigned int *)(DDR_PHY_CTRL_BASE + 0x1C8) = 0x7DU;
}



static inline void configure_ddr_io() {
    log_message(LOG_LEVEL_DEBUG, "Configuring DDR IO...\n");

    /* Configure DDR IO for drive strength, slew rate, and signal integrity */
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + 0x1404) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + 0x1408) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + 0x140C) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + 0x1440) = 0x16B;
    *(volatile uint32_t *)(CONTROL_MODULE_BASE + 0x1444) = 0x16B;


    /* Clear DDR IO Control bits for DDR3_RST_DEF_VAL ans DDR_WUCLK_DISABLE */
    *(volatile unsigned int *)(CONTROL_MODULE_BASE + 0xE04) &= 0xEFFFFFFF;


    /* CKE config */
    *(volatile unsigned int *)(CONTROL_MODULE_BASE + 0x131C) |= 0x00000001U;
    *(volatile unsigned int *)(SOC_EMIF_0_REGS + 0xE4) 	      = 0x06;
    *(volatile unsigned int *)(SOC_EMIF_0_REGS + 0xE4)       |= 0x00100000;
    *(volatile unsigned int *)(SOC_EMIF_0_REGS + 0xE8)        = 0x06;
    *(volatile unsigned int *)(SOC_EMIF_0_REGS + 0xE8)       |= 0x00100000;

    log_message(LOG_LEVEL_DEBUG, "DDR IO Configuration Complete!\n");
}


static inline void enable_core_pll(void)
{
    REG32_write_masked(CM_WKUP_BASE, 0x90, 0x7, 0x4);
    WAIT_FOR_REG32(CM_WKUP_BASE, 0x5c, 0x100, 0x100, TIMEOUT);

    /* set the multiplier and divider */
    *(volatile unsigned int *)(CM_WKUP_BASE + 0x68) = (1000 << 8) | 23;

    /* Set M4, M5, M6 */
    REG32_write_masked(CM_WKUP_BASE, 0x80, 0x0000001F, 10 << 0x00000000);
    REG32_write_masked(CM_WKUP_BASE, 0x84, 0x0000001F, 8 << 0x00000000);
    REG32_write_masked(CM_WKUP_BASE, 0xd8, 0x0000001F, 4 << 0);

    /* LOCK the PLL */

   *(volatile unsigned int *)(CM_WKUP_BASE + 0x90) |= DPLL_LOCK; 
    WAIT_FOR_REG32(CM_WKUP_BASE, 0x5c, 0x1, 0x1, TIMEOUT);
}



static inline void enable_ddr_pll(void)
{
    REG32_write_masked(CM_WKUP_BASE, 0x94, 0x7, 0x4);
    WAIT_FOR_REG32(CM_WKUP_BASE, 0x34, 0x100, 0x100, TIMEOUT);

    REG32_write_masked(CM_WKUP_BASE, 0x40, 0x0007FF00, 303 << 8);
    REG32_write_masked(CM_WKUP_BASE, 0x40, 0x0000007F, 23);

    /* Lock the PLL and wait for it */
   *(volatile unsigned int *)(CM_WKUP_BASE + CM_CLKMODE_DPLL_DDR) |= DPLL_LOCK; 


    WAIT_FOR_REG32(CM_WKUP_BASE, 0x34, 0x1, 0x1, TIMEOUT);
}


static inline void init_emif(void)
{
    uint32_t expected;
    REG32_write_masked(CM_PER_BASE, CM_PER_EMIF_CLKCTRL, 0x3U, 0x2U);

    expected = CM_PER_L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK |
               CM_PER_L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK;
    
    WAIT_FOR_REG32(CM_PER_BASE, 0x0C, expected, expected, TIMEOUT);
}



void setup_memory() { 
    
    enable_core_pll();
    
    enable_ddr_pll();
   
    enable_emif_clocks();
    init_emif();


    init_ddr3();
}

int test_ddr3_memory() {
    uint32_t *mem_addr;
    uint32_t read_back;
    uint32_t patterns[] = {0xAAAAAAAA, 0x55555555, 0x00000000, 0xFFFFFFFF};
    int errors = 0;
    int p;


    log_message(LOG_LEVEL_DEBUG, "DDR3 Memory Test Start\n");

    for (p = 0; p < 4; p++) {
        uint32_t test_pattern = patterns[p];


	for (mem_addr = (uint32_t *)DDR_START_ADDR;
			mem_addr < (uint32_t *)(DDR_START_ADDR + DDR_SIZE);
             mem_addr++) {
            *mem_addr = test_pattern;
            read_back = *mem_addr;

            if (read_back != test_pattern) {
                errors++; 
	    }
        }
	


    }

    if (errors > 0) {
        log_message(LOG_LEVEL_ERROR, "Memory Test Failed! Errors: %d \n", errors);
    } else {
        log_message(LOG_LEVEL_INFO, "Memory Test Passed!\n");
    }

    return errors;
}

