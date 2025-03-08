#include <uart.h>
#include <interrupt.h>
#include <circular_buffer.h>

circular_char_buffer_t uart0_rx_buffer;

void uart0_interrupt_init(void);

void uart_init(unsigned short uart_index,
    unsigned int baud_rate,
    unsigned short stop_bit_en,
    unsigned short num_stop_bits,
    unsigned short parity_en,
    unsigned short parity_type,
    unsigned short char_length) {

    (void)baud_rate;
    (void)stop_bit_en;
    (void)num_stop_bits;
    (void)parity_en;
    (void)parity_type;
    (void)char_length;

    /* Disable interrupts (IER) */
    UART0->IER_DLH = 0x00;

    /* Enable Divisor Latch Access (LCR[7] = 1) */
    UART0->LCR = 0x80;

    /* Set the baud rate (115200) */
    UART0->RBR_THR_DLL = 13;
    UART0->IER_DLH = 0x00;

    /* Configure 8N1 (8 data bits, no parity, 1 stop bit) */
    /* LCR[1:0] = 11 (8 bits), LCR[3] = 0 (no parity)*/
    UART0->LCR = 0x03;  

    /* Enable FIFO (FCR[0] = 1) */
    UART0->IIR_FCR = 0x01;

    circular_char_buffer_init(&uart0_rx_buffer);

    uart0_interrupt_init();
}
void uart_putc(char c) {
    // Wait until Transmit Holding Register is empty (LSR[5] = 1)
    while (!(UART0->LSR & (1 << 5)));
    UART0->RBR_THR_DLL = c;
}

void uart_puts(const char* str) {
    while (*str) {
        uart_putc(*str++);
    }
}

char uart_getc(void) {
    // Wait until Data Ready (LSR[0] = 1)
    while (!(UART0->LSR & 1));
    return UART0->RBR_THR_DLL;
}

void uart_handler(int irq, void *data) {
    (void)data;
    char c = UART0->RBR_THR_DLL;  // Read the character (clears interrupt)
    INTC->IRQ_PEND[0] = (irq << 1);
    uart_putc(c); // echo the character for now
}

void uart0_interrupt_init(void) {
    // Disable UART first
    UART0->IER_DLH = 0;

    // Configure line control (8N1)
    UART0->LCR = 0x03; // 8 bits, no parity, 1 stop bit

    // Enable FIFOs
    UART0->IIR_FCR = 0x01;

    // Enable receive interrupts
    UART0->IER_DLH = UART_IER_RX_INT;

    // Enable UART0 interrupt
    INTC_register_irq(UART0_INT_NUM, handle_uart0_irq);
    INTC_enable_irq(UART0_INT_NUM);
}


void handle_uart0_irq(void) {
    uart_handler(UART0_INT_NUM, NULL);
}

unsigned int uart0_getchar(char *c) {
    if(circular_char_buffer_pop(&uart0_rx_buffer, c)){
        return 1;
    }
    return 0;
}

unsigned int uart0_readline(char *buffer, unsigned int buffer_size) {
    char c;
    unsigned int i = 0;

    while (i < buffer_size) {
        if (uart0_getchar(&c)) {
            if (c == '\r') {
                buffer[i] = '\0';
                return i+1;
            } else {
                buffer[i] = c;
                i++;
            }
        }else{
            return i;
        }
    }
    return i;
}