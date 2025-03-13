#ifndef DDR_H
#define DDR_H

#include <stdint.h>

#define DDR_START_ADDR  0x80000000  /* Start of DDR3 memory */
#define DDR_SIZE        0x100000    /* 1MB test range */


static inline void configure_ddr_io();
static inline void configure_ddr_phy(void);
static inline void configure_ddr_phy_cmd_data(void);


#endif /* DDR_H */
