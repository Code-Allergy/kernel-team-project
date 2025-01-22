#ifndef MEMORY_MAP_BBB_H
#define MEMORY_MAP_BBB_H

//----------------------------------------//
//            BBB memory map             //
//----------------------------------------//
// UART0
#define UART0_BASE 0x44E09000
#define UART0_DR   (UART0_BASE + 0x00)

#define prcm_base  0x44e00000
#define io1_base   0x4804c000
#define io2_base   0x481ac000

#define IO_SYSCONFIG	0x010
#define IO_SYSSTATUS	0x114
#define IO_CONTROL	0x130

#define IO_nOE		0x134
#define IO_IN		0x138
#define IO_OUT		0x13c
#define IO_CLR		0x190
#define IO_SET		0x194

#define IO_FILTER_TIME	0x154
#define IO_FILTER_EN	0x150

#define IO_IRQ_ON_LOW	0x140
#define IO_IRQ_ON_HIGH	0x144
#define IO_IRQ_ON_RISE	0x148
#define IO_IRQ_ON_FALL	0x14c

#define IO_IRQ0		0x02c
#define IO_IRQ0_SET	0x024
#define IO_IRQ0_CLR	0x02c
#define IO_IRQ0_EN	0x034
#define IO_IRQ0_DIS	0x03c

#define IO_IRQ1		0x030
#define IO_IRQ1_SET	0x028
#define IO_IRQ1_CLR	0x030
#define IO_IRQ1_EN	0x038
#define IO_IRQ1_DIS	0x040

#define CLK_L4LS	0x000
#define MOD_GPIO1	0x0ac	// debounce clock enable in bit 18
#define MOD_GPIO2	0x0b0	// debounce clock enable in bit 18
#define MOD_GPIO3	0x0b4	// debounce clock enable in bit 18


// for handler mode can use:
//	0b11111  system mode (same stack as thread mode)
//	0b10011  supervisor mode (separate stack, not supported right now)
//
#define MODE_USR 0b10000	// thread-mode, unprivileged
#define MODE_THR 0b11111	// thread-mode, privileged
#define MODE_HND 0b11111	// handler-mode (always privileged)


#endif