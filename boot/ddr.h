#ifndef DDR_H
#define DDR_H

#include <stdint.h>

#define CM_PER_EMIF_CLKCTRL  0x44E00028

#define EMIF_BASE  0x4C000000
#define EMIF_DDR_PHY_CTRL_1  (EMIF_BASE + 0xE4)
#define EMIF_SDRAM_CONFIG    (EMIF_BASE + 0x8)
#define EMIF_SDRAM_CONFIG_2  (EMIF_BASE + 0xC)
#define EMIF_SDRAM_REF_CTRL  (EMIF_BASE + 0x10)
#define EMIF_SDRAM_TIM_1     (EMIF_BASE + 0x18)

#define CM_PER_L3_CLKCTRL        0x44E000E0
#define CM_PER_L3S_CLKCTRL       0x44E000E4
#define CM_PER_L4LS_CLKCTRL      0x44E00060
#define CM_PER_EMIF_CLKCTRL      0x44E00028


#define CM_DPLL_DDR              0x44E00510  // DDR PLL control
#define CM_DPLL_DDR_LOCK         0x44E00514  // DDR PLL lock status
#define CM_DPLL_DDR_FREQSEL      0x44E0051C  // DDR PLL Frequency Select
#define CM_DPLL_DDR_DIV    0x44E00518  // Divider control
#define CM_DPLL_DDR_MN     0x44E0051C  // Multiplier/Divider control

#define DDR_START_ADDR  0x80000000  /* Start of DDR3 memory */
#define DDR_SIZE        0x100000    /* 1MB test range */

void init_ddr3();
void enable_emif_clocks();
void reset_emif_ddr3();
void setup_memory();
int test_ddr3_memory();
void precharge_ddr3();
void iota();

#endif /* DDR_H */

