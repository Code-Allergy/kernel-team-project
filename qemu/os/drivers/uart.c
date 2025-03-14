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
    char c = UART0->RBR_THR_DLL;  // Read the character (clears interrupt)
    INTC->IRQ_PEND[0] = (UART0_INT_NUM << 1);
    circular_char_buffer_push(&uart0_rx_buffer, c);
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


void print_number(int32_t num, char base, bool is_signed) {
    char buffer[32];        /* Buffer to hold the number string */
    char *ptr = buffer;     /* Pointer to traverse the buffer */
    char *ptr1 = buffer;    /* Pointer for reversing the string */
    char tmp_char;
    uint32_t temp_num;  /* Use unsigned int to handle negatives in hex */
    int is_negative = 0;

    if (num == 0) {
        uart_putc('0');
        return;
    }

    /* Handle negative numbers for base 10 */
    if (num < 0 && base == 10 && is_signed) {
        is_negative = 1;
        temp_num = -num; /* Convert to positive for processing */
    } else {
        temp_num = (uint32_t) num;
    }

    /* Convert number to string */
    while (temp_num > 0) {
        *ptr++ = "0123456789abcdef"[temp_num % base];
        temp_num /= base;
    }

    if (is_negative) {
        *ptr++ = '-';  /* Add negative sign for decimal numbers */
    }

    *ptr-- = '\0'; /* Null-terminate */

    /* Reverse the string */
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }

    uart_puts(buffer); /* Output the number string */
}

void uart_vprintf(const char* fmt, va_list ap) {
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd': {
                    print_number(va_arg(ap, int), 10, true);
                    break;
                }
                case 'u': {
                    print_number(va_arg(ap, uint32_t), 10, false);
                    break;
                }
                case 'x': {
                    print_number(va_arg(ap, int), 16, true);
                    break;
                }
                case 's': {
                    uart_puts(va_arg(ap, char *));
                    break;
                }
                case 'c': {
                    uart_putc(va_arg(ap, int));
                    break;
                }
                default: {
                    uart_putc(*fmt);
                    break;
                }
            }
        } else {
            uart_putc(*fmt);
        }
        fmt++;
    }
}


void uart_printf(const char *format, ...) {
    va_list ap;
	va_start(ap, format);
	uart_vprintf(format, ap);
	va_end(ap);
}
