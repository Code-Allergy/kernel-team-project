#include <memory_map.h>
#include "uart.h"
#include "gpio.h"


// Base addresses (specific to AM335x)
#define UART0_BASE   0x44E09000  // UART0 base address
#define CM_WKUP      0x44E00400  // Clock Module Wakeup register base
#define L4LS_BASE    0x44E00000  // L4LS clock domain base

// General Registers offests
#define CLK_L4LS	0x000
#define CM_WKUP_UART0_CLKCTRL   0xB4


// UART Register Offsets
#define UART_SYSC    0x54  // UART System Configuration Register
#define UART_SYSS    0x58  // UART System Status Register

#define UART_LCR     0x0C  // UART Line Control Register
#define UART_EFR     0x08  // UART Enhanced Feature Register
#define UART_MCR     0x10  // UART Modem Control Register
#define UART_FCR     0x08  // UART FIFO Control Register (Write-only)
#define UART_TLR     0x1C  // UART FIFO Triggers Control register
#define UART_SCR     0x40  // UART Supplementary Control Register
#define UART_LSR     0x14  // UART interupt register
#define UART_THR     0x00  // UART Base register to write to 
#define UART_MDR1    0x20  // UART Mode Definition Register
#define UART_IER     0x04  // UART Interrupt Enable Register
#define UART_DLL     0x00  // UART Divisor Latch LSB Register
#define UART_DLH     0x04  // UART Divisor Latch MSB Register

#define REG32(addr) (*(volatile unsigned int *)(addr))


void switch_to_mode_b(void) {
    REG32(UART0_BASE + UART_LCR) = 0x00BF;  // Set UART_LCR to 0x00BF for Mode B
}


void switch_to_mode_a(void) {
    REG32(UART0_BASE + UART_LCR) = 0x0080;  // Set UART_LCR to 0x0000 for operational mode
}

int d = 1;

void uart_init(void) {
    GPIO_set(GPIO1_BASE, d<<21);
/*
    // Ensure the L4LS clock domain is active
    //unsigned int l4ls_status = REG32(L4LS_BASE + CLK_L4LS);
    REG32(L4LS_BASE + CLK_L4LS) |= 1<<10;
    //while ((REG32(L4LS_BASE + CLK_L4LS) & (1 << 10)) == 0);  // Wait for CLKACTIVITY_UART_GFCLK
    //GPIO_clear(GPIO1_BASE, 0xF<<21);
    //GPIO_set(GPIO1_BASE, ++d<<21);
    
    // Perform UART Software Reset
    REG32(UART0_BASE + UART_SYSC) |= (1 << 1);  // Set SOFTRESET bit
    while ((REG32(UART0_BASE + UART_SYSS) & 0x1) == 0);  // Wait for RESETDONE bit to be set
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    
    //switch_to_mode_b();
    unsigned int saved_lcr = REG32(UART0_BASE + UART_LCR);
    REG32(UART0_BASE + UART_LCR) = 0xBF;  // Set UART_LCR to 0xBF for Mode B

    unsigned int saved_enhanced_en = REG32(UART0_BASE + UART_EFR) & (1 << 4);  // Save the original ENHANCED_EN value
    REG32(UART0_BASE + UART_EFR) |= (1 << 4);  // Enable TCR_TLR submode (part 1 of 2)

    REG32(UART0_BASE + UART_LCR) = 0x80;  // Set UART_LCR to 0xBF for Mode A

    unsigned int saved_mcr = REG32(UART0_BASE + UART_MCR) & (1<<6);  // Save the original MCR value
    REG32(UART0_BASE + UART_MCR) |= (1 << 6);  // Enable TCR_TLR submode (part 2 of 2)

    // Enable FIFO mode, disable DMA mode, disable interrupt mode
    REG32(UART0_BASE + UART_FCR) = 0x1;  // Enable FIFO mode
    //REG32(UART0_BASE + UART_TLR) = 0x0;  // Set RX FIFO trigger to 0


    REG32(UART0_BASE + UART_LCR) = 0xBF;  // Set UART_LCR to 0xBF for Mode B


    // 9. restore the UART_EFR[4] ENHANCED_EN value
    if (saved_enhanced_en) {
        REG32(UART0_BASE + UART_EFR) |= (1 << 4);  // Set the ENHANCED_EN bit if it was not set
    }else{
        REG32(UART0_BASE + UART_EFR) &= ~(1 << 4);  // Clear the ENHANCED_EN bit if it was not set
    }

    // 10 switch to mode A
    REG32(UART0_BASE + UART_LCR) = 0x80;  // Set UART_LCR to 0xBF for Mode A

    // 11. restore the UART_MCR[6] TCR_TLR value
    if (saved_mcr) {
        REG32(UART0_BASE + UART_MCR) |= (1 << 6);  // Set the TCR_TLR bit if it was not set
    }else{
        REG32(UART0_BASE + UART_MCR) &= ~(1 << 6);  // Clear the TCR_TLR bit if it was not set
    }

    // 12. restore the UART_LCR value
    REG32(UART0_BASE + UART_LCR) = saved_lcr;


    // Set the baud rate to 115200
    //1.  disable UART to set baud rate
    REG32(UART0_BASE + UART_MDR1) |= 0x7;  // Set MDR1 to 0x7 for disable mode

    //2.  switch to mode B
    REG32(UART0_BASE + UART_LCR) = 0xBF;  // Set UART_LCR to 0xBF for Mode B

    //3. Enable access to UARTi.IER[7:4]
    REG32(UART0_BASE + UART_EFR) |= (1 << 4);  // Enable access to UARTi.IER[7:4]

    //4. Switct to register opraional mode to accedd UARTi.IER register
    REG32(UART0_BASE + UART_LCR) = 0x00;  // Set UART_LCR to 0x00 for operational mode

    //5a. Clear the UARTi.IER[4] bit to disable sleep mode
    REG32(UART0_BASE + UART_IER) &= ~(1 << 4);  // Clear the UART_IER[4] bit to disable sleep mode

    //5b. set the UARTi.IER bits to 0x0000
    REG32(UART0_BASE + UART_IER) = 0x0;  // Set the UART_IER bits to 0x0000

    //6. Switch to mode B
    REG32(UART0_BASE + UART_LCR) = 0xBF;  // Set UART_LCR to 0xBF for Mode B

    //7. Load the divertor register with the divisor value
    // using baud rate 115200
    REG32(UART0_BASE + UART_DLL) = 0x45;  // Set UART_DLL to 0x45 for 115200 baud rate
    REG32(UART0_BASE + UART_DLH) = 0x0;  // Set UART_DLH to 0x0 for 115200 baud rate

    //8. Switch to operational mode
    REG32(UART0_BASE + UART_LCR) = 0x00;  // Set UART_LCR to 0x00 for operational mode

    //9. interrupt configuration. We are not using interrupts

    //10. switch to mode B
    REG32(UART0_BASE + UART_LCR) = 0xBF;  // Set UART_LCR to 0xBF for Mode B
    //11. restore the UART_EFR[4] ENHANCED_EN value
    if (saved_enhanced_en) {
        REG32(UART0_BASE + UART_EFR) |= (1 << 4);  // Set the ENHANCED_EN bit if it was not set
    }else{
        REG32(UART0_BASE + UART_EFR) &= ~(1 << 4);  // Clear the ENHANCED_EN bit if it was not set
    }

    //12. Load the protocol register with the desired value
    //a. set LCR[7] to 0
    REG32(UART0_BASE + UART_LCR) &= ~(1 << 7);  // Clear the LCR[7] bit
    //b. set LCR[6] to 0
    REG32(UART0_BASE + UART_LCR) &= ~(1 << 6);  // Clear the LCR[6] bit


    //c. set parity, stop bits, and character length to desired values compatible with minicom  (8N1)
    // char length = 8 bits in UART_LCR[1:0]
    REG32(UART0_BASE + UART_LCR) |= 0x3;  // Set UART_LCR[1:0] to 0x3 for 8-bit character length

    //13. Set Uart mode in MDR1
    REG32(UART0_BASE + UART_MDR1) &= ~(0x7);  // Set MDR1 to 0x0 for UART16x mode

*/


    // Ungate UART0 Clock
    // REG32(L4LS_BASE + CLK_L4LS) |= 1<<10;
    REG32(CM_WKUP + CM_WKUP_UART0_CLKCTRL) = 0x2;  // Enable UART0 functional clock
    while ((REG32(CM_WKUP + CM_WKUP_UART0_CLKCTRL) & 0x3) != 0x2);  // Wait for idle state
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    // Save the original UART_LCR value

    unsigned int saved_enhanced_en = REG32(UART0_BASE + UART_EFR) & (1 << 4);  // Save the original ENHANCED_EN value
    REG32(UART0_BASE + UART_EFR) |= (1 << 4);  // Enable TCR_TLR submode (part 1 of 2)
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    // Save the original TLR value
    unsigned int saved_tlr = REG32(UART0_BASE + UART_TLR);

    switch_to_mode_a();
    unsigned int saved_mcr = REG32(UART0_BASE + UART_MCR);  // Save the original MCR value
    REG32(UART0_BASE + UART_MCR) |= (1 << 6);  // Enable TCR_TLR submode (part 2 of 2)
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    unsigned int fifo_config = 0;
    fifo_config |= (0x3 << 6);  // RX_FIFO_TRIG: Set to maximum,        load the new FIFO triggers (part 1 of 3)
    fifo_config |= (0x3 << 4);  // TX_FIFO_TRIG: Set to maximum,        load the new FIFO triggers (part 1 of 3)
    fifo_config |= (1 << 3);    // DMA_MODE: Enable DMA mode,           New DMA mode (part 1 of 2)
    fifo_config |= (1 << 0);    // FIFO_ENABLE: Enable the FIFO
    REG32(UART0_BASE + UART_FCR) = fifo_config;


    switch_to_mode_b();
    unsigned int tlr_config = 0;
    tlr_config |= (0xF << 4);  // RX_FIFO_TRIG_DMA,                     Load the new FIFO triggers (part 2 of 3)
    tlr_config |= (0xF << 0);  // TX_FIFO_TRIG_DMA,                     Load the new FIFO triggers (part 2 of 3)
    REG32(UART0_BASE + UART_TLR) = tlr_config;  // Write configuration to UART_TLR
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    unsigned int scr_config = 0;
    scr_config |= (1 << 7);    // RX_TRIG_GRANU1,                       Load the new FIFO triggers (part 3 of 3)
    scr_config |= (1 << 6);    // TX_TRIG_GRANU1,                       Load the new FIFO triggers (part 3 of 3)
    scr_config |= (0x3 << 1);  // DMA_MODE_2: Enable both modes         DMA mode (part 2 of 2)
    scr_config |= (1 << 0);    // DMA_MODE_CTL: Enable DMA control      DMA mode (part 2 of 2)
    REG32(UART0_BASE + UART_SCR) = scr_config;

    // Restore the UART_EFR[4] ENHANCED_EN value
    if (!saved_enhanced_en) {
        REG32(UART0_BASE + UART_EFR) &= ~(1 << 4);  // Clear the ENHANCED_EN bit if it was not set
    }
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    switch_to_mode_a();
    REG32(UART0_BASE + UART_MCR) = saved_mcr; // Restore the UART_MCR[6] TCR_TLR value
    REG32(UART0_BASE + UART_LCR) = 0x0; // Restore the UART_LCR value
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);

}


// Function to Transmit a Character
void uart_putc(char c) {
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    while (!(REG32(UART0_BASE + UART_LSR) & (1 << 5))) {
        // Busy-wait until Transmitter Holding Register is empty
    }
    GPIO_clear(GPIO1_BASE, 0xF<<21);
    GPIO_set(GPIO1_BASE, ++d<<21);
    // Write the character to the Transmit Holding Register (THR)
    REG32(UART0_BASE + UART_THR) = c;
}

// Function to Transmit a Null-Terminated String
void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}