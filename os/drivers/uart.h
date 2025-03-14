#ifndef UART_H
#define UART_H
#include <types.h>

/* UART Base adresses */
enum UartBase
{
    UART0_BASE = 0x44E09000,
    UART1_BASE = 0x48022000,
    UART2_BASE = 0x48024000,
    UART3_BASE = 0x481A6000,
    UART4_BASE = 0x481A8000,
    UART5_BASE = 0x481AA000
};

enum UartOperationMode
{
    UART_MODE_A = 0x80,
    UART_MODE_B = 0xBF
};
/* UART Register Offsets by mode */
// Page 4337 of the TRM
/*
0h THR Transmit Holding Register Section 19.5.1.1
0h RHR Receiver Holding Register Section 19.5.1.2
0h DLL Divisor Latches Low Register Section 19.5.1.3
4h IER_IRDA Interrupt Enable Register (IrDA) Section 19.5.1.4
4h IER_CIR Interrupt Enable Register (CIR) Section 19.5.1.5
4h IER_UART Interrupt Enable Register (UART) Section 19.5.1.6
4h DLH Divisor Latches High Register Section 19.5.1.7
8h EFR Enhanced Feature Register Section 19.5.1.8
8h IIR_UART Interrupt Identification Register (UART) Section 19.5.1.9
8h IIR_CIR Interrupt Identification Register (CIR) Section 19.5.1.10
8h FCR FIFO Control Register Section 19.5.1.11
8h IIR_IRDA Interrupt Identification Register (IrDA) Section 19.5.1.12
Ch LCR Line Control Register Section 19.5.1.13
10h MCR Modem Control Register Section 19.5.1.14
10h XON1_ADDR1 XON1/ADDR1 Register Section 19.5.1.15
14h XON2_ADDR2 XON2/ADDR2 Register Section 19.5.1.16
14h LSR_CIR Line Status Register (CIR) Section 19.5.1.17
14h LSR_IRDA Line Status Register (IrDA) Section 19.5.1.18
14h LSR_UART Line Status Register (UART) Section 19.5.1.19
18h TCR Transmission Control Register Section 19.5.1.20
18h MSR Modem Status Register Section 19.5.1.21
18h XOFF1 XOFF1 Register Section 19.5.1.22
1Ch SPR Scratchpad Register Section 19.5.1.23
1Ch TLR Trigger Level Register Section 19.5.1.24
1Ch XOFF2 XOFF2 Register Section 19.5.1.25
20h MDR1 Mode Definition Register 1 Section 19.5.1.26
24h MDR2 Mode Definition Register 2 Section 19.5.1.27
28h TXFLL Transmit Frame Length Low Register Section 19.5.1.28
28h SFLSR Status FIFO Line Status Register Section 19.5.1.29
2Ch RESUME RESUME Register Section 19.5.1.30
2Ch TXFLH Transmit Frame Length High Register Section 19.5.1.31
30h RXFLL Received Frame Length Low Register Section 19.5.1.32
30h SFREGL Status FIFO Register Low Section 19.5.1.33
34h SFREGH Status FIFO Register High Section 19.5.1.34
34h RXFLH Received Frame Length High Register Section 19.5.1.35
38h BLR BOF Control Register Section 19.5.1.36
38h UASR UART Autobauding Status Register Section 19.5.1.37
3Ch ACREG Auxiliary Control Register Section 19.5.1.38
40h SCR Supplementary Control Register Section 19.5.1.39
44h SSR Supplementary Status Register Section 19.5.1.40
48h EBLR BOF Length Register Section 19.5.1.41
50h MVR Module Version Register Section 19.5.1.42
54h SYSC System Configuration Register Section 19.5.1.43
58h SYSS System Status Register Section 19.5.1.44
5Ch WER Wake-Up Enable Register Section 19.5.1.45
60h CFPS Carrier Frequency Prescaler Register Section 19.5.1.46
64h RXFIFO_LVL Received FIFO Level Register Section 19.5.1.47
68h TXFIFO_LVL Transmit FIFO Level Register Section 19.5.1.48
6Ch IER2 IER2 Register Section 19.5.1.49
70h ISR2 ISR2 Register Section 19.5.1.50
74h FREQ_SEL FREQ_SEL Register Section 19.5.1.51
80h MDR3 Mode Definition Register 3 Section 19.5.1.52
84h TX_DMA_THRESHOLD TX DMA Threshold Register Section 19.5.1.53
*/
enum UartRegister
{
    UART_THR_OFF        = 0x00,      // Transmit Holding Register
    UART_RHR_OFF        = 0x00,      // Receiver Holding Register
    UART_DLL_OFF        = 0x00,      // Divisor Latches Low Register
    UART_IER_IRDA_OFF   = 0x04,      // Interrupt Enable Register (IrDA)
    UART_IER_CIR_OFF    = 0x04,      // Interrupt Enable Register (CIR)
    UART_IER_UART_OFF   = 0x04,      // Interrupt Enable Register (UART)
    UART_DLH_OFF        = 0x04,      // Divisor Latches High Register
    UART_EFR_OFF        = 0x08,      // Enhanced Feature Register
    UART_IIR_UART_OFF   = 0x08,      // Interrupt Identification Register (UART)
    UART_IIR_CIR_OFF    = 0x08,      // Interrupt Identification Register (CIR)
    UART_FCR_OFF        = 0x08,      // FIFO Control Register
    UART_IIR_IRDA_OFF   = 0x08,      // Interrupt Identification Register (IrDA)
    UART_LCR_OFF        = 0x0C,      // Line Control Register
    UART_MCR_OFF        = 0x10,      // Modem Control Register
    UART_XON1_ADDR1_OFF = 0x10,      // XON1/ADDR1 Register
    UART_XON2_ADDR2_OFF = 0x14,      // XON2/ADDR2 Register
    UART_LSR_CIR_OFF    = 0x14,      // Line Status Register (CIR)
    UART_LSR_IRDA_OFF   = 0x14,      // Line Status Register (IrDA)
    UART_LSR_UART_OFF   = 0x14,      // Line Status Register (UART)
    UART_TCR_OFF        = 0x18,      // Transmission Control Register
    UART_MSR_OFF        = 0x18,      // Modem Status Register
    UART_XOFF1_OFF      = 0x18,      // XOFF1 Register
    UART_SPR_OFF        = 0x1C,      // Scratchpad Register
    UART_TLR_OFF        = 0x1C,      // Trigger Level Register
    UART_XOFF2_OFF      = 0x1C,      // XOFF2 Register
    UART_MDR1_OFF       = 0x20,      // Mode Definition Register 1
    UART_MDR2_OFF       = 0x24,      // Mode Definition Register 2
    UART_TXFLL_OFF      = 0x28,      // Transmit Frame Length Low Register
    UART_SFLSR_OFF      = 0x28,      // Status FIFO Line Status Register
    UART_RESUME_OFF     = 0x2C,      // RESUME Register
    UART_TXFLH_OFF      = 0x2C,      // Transmit Frame Length High Register
    UART_RXFLL_OFF      = 0x30,      // Received Frame Length Low Register
    UART_SFREGL_OFF     = 0x30,      // Status FIFO Register Low
    UART_SFREGH_OFF     = 0x34,      // Status FIFO Register High
    UART_RXFLH_OFF      = 0x34,      // Received Frame Length High Register
    UART_BLR_OFF        = 0x38,      // BOF Control Register
    UART_UASR_OFF       = 0x38,      // UART Autobauding Status Register
    UART_ACREG_OFF      = 0x3C,      // Auxiliary Control Register
    UART_SCR_OFF        = 0x40,      // Supplementary Control Register
    UART_SSR_OFF        = 0x44,      // Supplementary Status Register
    UART_EBLR_OFF       = 0x48,      // BOF Length Register
    UART_MVR_OFF        = 0x50,      // Module Version Register
    UART_SYSC_OFF       = 0x54,      // System Configuration Register
    UART_SYSS_OFF       = 0x58,      // System Status Register
    UART_WER_OFF        = 0x5C,      // Wake-Up Enable Register
    UART_CFPS_OFF       = 0x60,      // Carrier Frequency Prescaler Register
    UART_RXFIFO_LVL_OFF = 0x64,      // Received FIFO Level Register
    UART_TXFIFO_LVL_OFF = 0x68,      // Transmit FIFO Level Register
    UART_IER2_OFF       = 0x6C,      // IER2 Register
    UART_ISR2_OFF       = 0x70,      // ISR2 Register
    UART_FREQ_SEL_OFF   = 0x74,      // FREQ_SEL Register
    UART_MDR3_OFF       = 0x80,      // Mode Definition Register 3
    UART_TX_DMA_THRESHOLD_OFF = 0x84 // TX DMA Threshold Register
};

/**
 * @brief Initializes a UART port
 *
 * @param uart_index    Index of the UART to initialize.
 * @param baud_rate     Desired baud rate for UART communication.
 * @param stop_bit_en   Enable or disable stop bits.
 * @param num_stop_bits Number of stop bits to use (0= 1 top bit, 1 = 1.5 stop
 bits).
 * @param parity_en     Enable or disable parity checking.
 * @param parity_type   Type of parity (0 for even, 1 for odd ignored if parity
 is disabled).
 * @param char_length   Length of the character (number of data bits).
 *
 * Function Call:
 *
 uart_init(
    0,         // UART index (0 = UART0, 1 = UART1, etc.)
    115200,    // Baud rate for communication
    1,         // Stop bit enable (1 = enabled, 0 = disabled)
    0,         // Number of stop bits (0 = 1 stop bit, 1 = 1.5/2 stop bits)
    0,         // Parity enable (1 = enabled, 0 = disabled)
    0,         // Parity type (0 = even, 1 = odd; ignored if parity is disabled)
    3          // Character length
 );
 */
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

void print_number(int32_t num, char base, bool is_signed);
void uart_printf(const char* format, ...);
void uart_vprintf(const char* fmt, va_list ap);

/* Both non blocking*/
/**
 * @brief Get a single character from UART0's buffer.'
 *
 * @param c Pointer to a character variable where the received character will be
 * stored.
 * @return Returns 1 if a character is present and 0 otherwise.
 */
unsigned int uart0_getchar(char* c);

/**
 * @brief Reads a line of text from UART0's buffer.
 *
 * NOTE: The destination buffer must be large enough to store the received line,
 * including the newline character and the null terminator. The function will
 * read characters from the buffer until a newline character is reached
 * or the destination buffer is full (in which case the destination buffer
 * will not have a newline and null terminator!!).
 *
 * @param buffer Pointer to a character array where the received line will be
 * stored.
 * @param buffer_size The size of the buffer.
 * @return Returns the number of characters read, or a non-zero error code on
 * failure.
 */
unsigned int uart0_readline(char* buffer, unsigned int buffer_size);

#endif
