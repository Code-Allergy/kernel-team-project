#ifndef UART_H
#define UART_H

#include <types.h>

#define UART0_BASE (0x01C28000)
#define UART4_BASE (0x01c29000)

typedef struct {
    volatile uint32_t RBR_THR_DLL; /* 0x00: Receive/Transmit Buffer */
    volatile uint32_t IER_DLH;     /* 0x04: Interrupt Enable */
    volatile uint32_t IIR_FCR;     /* 0x08: Interrupt Identification */
    volatile uint32_t LCR;         /* 0x0C: Line Control */
    volatile uint32_t MCR;         /* 0x10: Modem Control */
    volatile uint32_t LSR;         /* 0x14: Line Status */
    volatile uint32_t MSR;         /* 0x18: Modem Status */
    volatile uint32_t SCR;         /* 0x1C: Scratch */
} UART_Reg;

#define UART0 ((UART_Reg *)UART0_BASE)
#define UART_IER_RX_INT 0x01

void uart_init(unsigned short uart_index,
    unsigned int baud_rate,
    unsigned short stop_bit_en,
    unsigned short num_stop_bits,
    unsigned short parity_en,
    unsigned short parity_type,
    unsigned short char_length);
void uart_putc(char c);
void uart_puts(const char* str);

char uart_getc(void);

void uart0_interrupt_init(void);
void handle_uart0_irq(void);

/* Both non blocking*/
unsigned int uart0_getchar(char *c);
unsigned int uart0_readline(char *buffer, unsigned int buffer_size);

#endif 