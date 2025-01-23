#ifndef MEMORY_MAP_QEMU_H
#define MEMORY_MAP_QEMU_H

//----------------------------------------//
//            Qemu memory map             //
//----------------------------------------//

#define PRCM_BASE                       0x44e00000
#define CONTROL_MODULE                  0x44E10000

// UART0

// TODO: Add the memory map for the QEMU platform. These are all wrong.

/* UART0 Registers */
#define UART0_BASE                      0x44E09000
#define UART0_THR                       *(volatile unsigned int *)(UART0_BASE + 0x00)  // Transmit Holding Register
#define UART0_LSR                       *(volatile unsigned int *)(UART0_BASE + 0x14)  // Line Status Register
#define UART0_LCR                       *(volatile unsigned int *)(UART0_BASE + 0x0C)  // Line Control Register
#define UART0_DLL                       *(volatile unsigned int *)(UART0_BASE + 0x00)  // Divisor Latch Low
#define UART0_DLH                       *(volatile unsigned int *)(UART0_BASE + 0x04)  // Divisor Latch High
#define UART0_SYSC                      *(volatile unsigned int *)(UART0_BASE + 0x54)  // System Configuration

/* Clock and Pinmux Registers */
#define CM_PER_UART0_CLKCTRL            *(volatile unsigned int *)(PRCM_BASE + 0x6C) // UART0 Clock Control
#define PINMUX_UART0_RXD                *(volatile unsigned int *)(CONTROL_MODULE + 0x954)
#define PINMUX_UART0_TXD                *(volatile unsigned int *)(CONTROL_MODULE + 0x950)

/* UART0 Line Status Register Bits */
#define UART_LSR_THRE                   (1 << 5)  // Transmitter Holding Register Empty

#define GPIO1_BASE                      0x4804c000
#define GPIO2_BASE                      0x481ac000

#define GPIO_SYSCONFIG	                0x010
#define GPIO_SYSSTATUS	                0x114
#define GPIO_CONTROL	                0x130

#define GPIO_nOE		                0x134
#define GPIO_IN		                    0x138
#define GPIO_OUT		                0x13c
#define GPIO_CLR		                0x190
#define GPIO_SET		                0x194

#define GPIO_FILTER_TIME	            0x154
#define GPIO_FILTER_EN	                0x150

#define GPIO_IRQ_ON_LOW	                0x140
#define GPIO_IRQ_ON_HIGH	            0x144
#define GPIO_IRQ_ON_RISE	            0x148
#define GPIO_IRQ_ON_FALL	            0x14c

#define GPIO_IRQ0		                0x02c
#define GPIO_IRQ0_SET	                0x024
#define GPIO_IRQ0_CLR	                0x02c
#define GPIO_IRQ0_EN	                0x034
#define GPIO_IRQ0_DIS	                0x03c

#define GPIO_IRQ1		                0x030
#define GPIO_IRQ1_SET	                0x028
#define GPIO_IRQ1_CLR	                0x030
#define GPIO_IRQ1_EN	                0x038
#define GPIO_IRQ1_DIS	                0x040

#define CLK_L4LS	                    0x000
#define MOD_GPIO1	                    0x0ac	// debounce clock enable in bit 18
#define MOD_GPIO2	                    0x0b0	// debounce clock enable in bit 18
#define MOD_GPIO3	                    0x0b4	// debounce clock enable in bit 18


// for handler mode can use:
//	0b11111  system mode (same stack as thread mode)
//	0b10011  supervisor mode (separate stack, not supported right now)
//
#define MODE_USR 0b10000	// thread-mode, unprivileged
#define MODE_THR 0b11111	// thread-mode, privileged
#define MODE_HND 0b11111	// handler-mode (always privileged)

#endif