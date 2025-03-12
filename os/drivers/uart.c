#include <uart.h>
#include <utils.h>
#include <clock_module.h>
#include <interrupt.h>
#include <circular_buffer.h>

#define CONTROL_MODULE_BASE 0x44E10000
#define CONTROL_MODULE_UART0_RXD_OFF 0x970
#define CONTROL_MODULE_UART0_TXD_OFF 0x974

circular_char_buffer_t uart0_rx_buffer;

void uart_init( unsigned short uart_index,
                unsigned int    baud_rate, 
                unsigned short  stop_bit_en,
                unsigned short  num_stop_bits,
                unsigned short  parity_en, 
                unsigned short  parity_type, 
                unsigned short  char_length
            ) {
    unsigned int lcr, efr_bit4, mcr_bit6;

    // Only support UART0 for now
    switch (uart_index){
        case 0:
            // 0. Clock setup. Both module and interface clock must be enabled!!!
            // Enable UART0 module clock
            REG32_write_masked(CM_WKUP_BASE, CM_WKUP_UART0_CLKCTRL, 0x3, 0x2); // MODULEMODE = enable
            while (REG32_read(CM_WKUP_BASE, CM_WKUP_UART0_CLKCTRL) & (0x3 << 16)) {} // Wait for fully enabled

            // Enable UART0 interface clock (l4_wkup)
            REG32_write_masked(CM_WKUP_BASE, CM_WKUP_L4WKUP_CLKCTRL, 0x3, 0x2); // MODULEMODE = enable
            while (REG32_read(CM_WKUP_BASE, CM_WKUP_L4WKUP_CLKCTRL) & (0x3 << 16)) {} // Wait for fully enabled

            // 1. Pin muxing. This is required because many pins have multiple functions (e.g. UART, GPIO, SPI, etc.)
            // Control module pin muxing
            REG32_write(CONTROL_MODULE_BASE, CONTROL_MODULE_UART0_RXD_OFF, 0x30); // [3] = 0 for pull-up/pull-down enable, 
                                                                                  // [4] = 1 for pull-up select, 
                                                                                  // [5] = 1 for receiver enable
            REG32_write(CONTROL_MODULE_BASE, CONTROL_MODULE_UART0_TXD_OFF, 0x10); // [3] = 0 for pull-up/pull-down enable, 
                                                                                  // [4] = 1 for pull-up select,
                                                                                  // [5] = 0 for receiver disable (this is a TX pin)

            /* Now the steps described in the TRM (19.4.1.1)*/
            // UART software reset
            REG32_write_masked(UART0_BASE, UART_SYSC_OFF, 0x2, 0x2); // Initiate software reset
            while (!(REG32_read(UART0_BASE, UART_SYSS_OFF) & 0x1)) {} // Wait for reset to complete
            REG32_write(UART0_BASE, UART_SYSC_OFF, 0x8); // Set SYSC config

            /*-------------- 19.4.1.1.2 FIFOs and DMA Settings --------------- */
            // 1. Save LCR and switch to register configuration mode B
            lcr = REG32_read(UART0_BASE, UART_LCR_OFF);
            REG32_write(UART0_BASE, UART_LCR_OFF, 0xBF);

            // 2. Enable register submode TCR_TLR to access the UARTi.UART_TLR register (part 1 of 2):
            efr_bit4 = REG32_read_masked(UART0_BASE, UART_EFR_OFF, 0x10);
            REG32_write_masked(UART0_BASE, UART_EFR_OFF, 0x10, 0x10); // Set ENHANCEDEN = 1

            // 3. Switch to register configuration mode A to access the UARTi.UART_MCR register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0x80);

            // 4. Enable register submode TCR_TLR to access the UARTi.UART_TLR register (part 2 of 2)
            mcr_bit6 = REG32_read_masked(UART0_BASE, UART_MCR_OFF, 0x40);
            REG32_write_masked(UART0_BASE, UART_MCR_OFF, 0x40, 0x40); // Set TCR_TLR = 1

            // 5. Enable the FIFO; load the new FIFO triggers (part 1 of 3) and the new DMA mode (part 1 of 2)
            REG32_write(UART0_BASE, UART_FCR_OFF, 0x07);    // [0] FIFO_EN = 1
                                                            // [1] RX_FIFO_CLEAR = 1
                                                            // [2] TX_FIFO_CLEAR = 1
                                                            // [3] DMA_MODE = 0 (DMA mode disabled)
                                                            // [5-4] TX_FIFO_TRIG = 0x0 (8 characters)
                                                            // [7-6] RX_FIFO_TRIG = 0x0 (8 characters)
            
            // 6. Switch to register configuration mode B to access the UARTi.UART_EFR register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0xBF);

            // 7. Load the new FIFO triggers (part 2 of 3)
            // For TX SCR[6] = 0, and TLR[3] to TLR[0] = 0, then: Defined by FCR[5] and FCR[4] (either 8, 16, 32, 56 characters)
            // For RX SCR[7] = 0, and TLR[7] to TLR[4]=0, then: Defined by FCR[7] and FCR[6] (either 8, 16, 56, 60 characters).
            REG32_write(UART0_BASE, UART_TLR_OFF, 0x00);    // [3-0] RX_FIFO_TRIG_DMA = 0x00, use setting from FCR register
                                                            // [7-4] TX_FIFO_TRIG_DMA = 0x00, use setting from FCR register
            
            // 8. Load the new FIFO triggers (part 3 of 3) and the new DMA mode (part 2 of 2)
            REG32_write(UART0_BASE, UART_SCR_OFF, 0x00);    // [0] DMA_MODE_CTL = 0 (DMA set with FCR register)
                                                            // [2-1] DMAMODE2 = 0x0 (Has no effect when DMA_MODE_CTL = 0)
                                                            // [3] TXEMPTYCTL = 0 (Normal mode for THR interrupt)
                                                            // [4] RXCTSDSRWAKEUPENABLE = 0 (CTS/DSR wakeup disabled)
                                                            // [5] DSRIT = 0 (DSR interrupt disabled)
                                                            // [6] TXTRIGGRANU1 = 0 (disable TX trigger granularity)
                                                            // [7] RXTRIGGRANU1 = 0 (disable RX trigger granularity)
            // 9. Restore the UARTi.UART_EFR[4] ENHANCED_EN value saved in Step 2a
            REG32_write_masked(UART0_BASE, UART_EFR_OFF, 0x10, efr_bit4);

            // 10. Switch to register configuration mode A to access the UARTi.UART_MCR register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0x80);

            // 11. Restore the UARTi.UART_MCR[6] TCR_TLR value saved in Step 4a
            REG32_write_masked(UART0_BASE, UART_MCR_OFF, 0x40, mcr_bit6);

            // 12. Restore the UARTi.UART_LCR value saved in Step 1a
            REG32_write(UART0_BASE, UART_LCR_OFF, lcr);

            /* -------------- 19.4.1.1.3 Protocol, Baud Rate, and Interrupt Settings -----------*/
            // 1. Disable UART to access the UARTi.UART_DLL and UARTi.UART_DLH registers
            REG32_write_masked(UART0_BASE, UART_MDR1_OFF, 0x7, 0x7);    // Set MODE_SELECT = 0x7 (disable UART)

            // 2. Switch to register configuration mode B to access the UARTi.UART_EFR register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0xBF);

            // 3. Enable access to the UARTi.UART_IER[7:4] bit field
            efr_bit4 = REG32_read_masked(UART0_BASE, UART_EFR_OFF, 0x10);
            REG32_write_masked(UART0_BASE, UART_EFR_OFF, 0x10, 0x10); // Set ENHANCED_EN = 1

            // 4. Switch to register operational mode to access the UARTi.UART_IER register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0x00);

            // 5. Clear the UARTi.UART_IER register (set the UARTi.UART_IER[4] SLEEP_MODE bit to 0 to change
            //    the UARTi.UART_DLL and UARTi.UART_DLH registers). Set the UARTi.UART_IER register value to 0x0000
            REG32_write(UART0_BASE, UART_IER_UART_OFF, 0x00);

            // 6. Switch to register configuration mode B to access the UARTi.UART_DLL and UARTi.UART_DLH registers
            REG32_write(UART0_BASE, UART_LCR_OFF, 0xBF);

            // 7. Load the new divisor value
            // Baud rate = (UART module clock) / (16 * (DLL + DLH/256))
            // For 115200 baud rate, DLL = 0x1A, DLH = 0x00
            REG32_write(UART0_BASE, UART_DLL_OFF, 0x1A); // DLL = 0x1A
            REG32_write(UART0_BASE, UART_DLH_OFF, 0x00); // DLH = 0x00

            // 8. Switch to register operational mode to access the UARTi.UART_IER register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0x00);

            // 9. Load the new interrupt configuration (0: Disable the interrupt; 1: Enable the interrupt)
            // Enable receive holding register interrupt
            REG32_write(UART0_BASE, UART_IER_UART_OFF, 0x01);   // [0] RHRIT = 1 (Receive holding register interrupt)
                                                                // [1] THRIT = 0 (Tranmission holding register interrupt)
                                                                // [2] LINESTIT = 0 (receiver line status interrupt)
                                                                // [3] MODEMSTSIT = 0 (modem status register interrupt)
                                                                // [4] SLEEPMODE = 0 (Disables sleep mode)
                                                                // [5] XOFFIT = 0 (XOFF interrupt)
                                                                // [6] RTSIT = 0 (RTS (active-low) interrup)
                                                                // [7] CTSIT = 0 (CTS (active-low) interrupt)
            
            // 10. Switch to register configuration mode B to access the UARTi.UART_EFR register
            REG32_write(UART0_BASE, UART_LCR_OFF, 0xBF);

            // 11. Restore the UARTi.UART_EFR[4] ENHANCED_EN value saved in Step 3a
            REG32_write_masked(UART0_BASE, UART_EFR_OFF, 0x10, efr_bit4);

            // 12. Load the new protocol formatting (parity, stop-bit, character length) and switch to register operational mode
            REG32_write(UART0_BASE, UART_LCR_OFF, 
                (0 << 7) |                      // [7] DIV_EN = 0 (disable divisor latch access)
                (0 << 6) |                      // [6] BREAK_EN = 0 (disable break condition)
                (0 << 5) |                      // [5] PARITY_TYPE_2
                ((parity_type & 0x1) << 4) |    // [4] PARITY_TYPE_1
                ((parity_en & 0x1) << 3) |      // [3] PARITY_EN
                ((num_stop_bits & 0x1) << 2) |  // [2] NB_STOP
                ((char_length - 5) & 0x3)       // [1:0] CHAR_LENGTH
            );

            // 13. Load the new UART mode
            REG32_write(UART0_BASE, UART_MDR1_OFF, 0x0); // UART 16x mode

            /* Setup uart0 RHR interrup*/
            uart0_interrupt_init();

            /* initialize receive buffer*/
            circular_char_buffer_init(&uart0_rx_buffer);

            break;
        default:
            break;
    }

}

void uart_putc(char c) {

    if (c == '\n'){
        uart_putc('\r');
    }
    
    // Only support UART0 for now
    // Wait for the THR empty bit to be set
    while (!(REG32_read(UART0_BASE, UART_LSR_UART_OFF) & 0x20));
    // Write the character to the THR
    REG32_write(UART0_BASE, UART_THR_OFF, c);
}

void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}

char uart_getc(void) {
    // Only support UART0 for now
    // Wait for the RHR data ready bit to be set
    while (!(REG32_read(UART0_BASE, UART_LSR_UART_OFF) & 0x1));
    // Read the character from the RHR
    return REG32_read(UART0_BASE, UART_RHR_OFF);
}


void uart0_interrupt_init(void) {
    // Register the UART0 interrupt handler
    INTC_register_irq(UART0_INT_NUM, handle_uart0_irq);
    INTC_set_priority(UART0_INT_NUM, 1); /* Priority 0 is non maskable*/
    // Enable the UART0 interrupt
    INTC_enable_irq(UART0_INT_NUM);
}


void handle_uart0_irq(void) {
    /* Disable UART0 interrupts */
    REG32_write(UART0_BASE, UART_IER_UART_OFF, 0x00);

    /* Echo the character back */
    char c = uart_getc();
    if(c == '\r'){
        c = '\n';
    }
    uart_putc(c);

    /* Push the character to the buffer */
    circular_char_buffer_push(&uart0_rx_buffer, c);

    /* Re enable UART0 interrupts*/
    REG32_write(UART0_BASE, UART_IER_UART_OFF, 0x01);

    return;
}

/* Device functions */

unsigned int uart0_getchar(char *c) {
    if(circular_char_buffer_pop(&uart0_rx_buffer, c)){
        return 1;
    }
    return 0;
}

unsigned int uart0_readline(char *buffer, unsigned int buffer_size) {
    char c;
    unsigned int i = 0;

    if(uart0_rx_buffer.lines == 0){
        return 0;
    }

    while (i < buffer_size - 1) {
        if (uart0_getchar(&c)) {
            buffer[i++] = c;
            if (c == '\n') {
                break;
            }
        } else {
            break;
        }
    }
    buffer[i] = '\0';
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


void uart_printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);

    while (*format) {
        if (*format == '%') {
            format++;
            switch (*format) {
                case 'd': {
                    print_number(va_arg(ap, int), 10 , true);
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
                    uart_putc(*format);
                    break;
                }
            }
        } else {
            uart_putc(*format);
        }
        format++;
    }

    va_end(ap);
}