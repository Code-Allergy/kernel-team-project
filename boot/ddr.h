#ifndef DDR_H
#define DDR_H

#include <stdint.h>

#define DDR_START_ADDR  0x80000000  /* Start of DDR3 memory */
#define DDR_SIZE        0x100000    /* 1MB test range */


void init_ddr3();
void enable_emif_clocks();
void reset_emif_ddr3();
void setup_memory();
int test_ddr3_memory();
void precharge_ddr3();
void iota();
void configure_ddr_phy();
void configure_ddr_phy_cmd_data();
void configure_ddr_io();
void configure_emif_zq();

#endif /* DDR_H */

