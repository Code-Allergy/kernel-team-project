#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#include <types.h>
#include <utils.h>

#define INTC_BASE_ADDR              0x48200000
#define INTC_CONTROL_OFF            0x48
#define INTC_SIR_IRQ_OFF            0x40
#define INTC_SYSCONFIG_OFF          0x10
#define INTC_IDLE_OFF               0x50
#define INTC_THRESHOLD_OFF          0x68
#define INTC_STATUS_OFF             0x14
#define INTC_IRQ_REG_BIT(irq_num)   (1 << ((irq_num) % 32))
#define INTC_IRQ_REG_NUM(irq_num)   ((irq_num) / 32) /* Either 0, 1, 2, or 3*/
#define INTC_ILEn_OFF(irq_num)      (0x100 + ((irq_num) * 4))
#define INTC_MIRn_OFF(irq_num)      (0x84 + ((8*(INTC_IRQ_REG_NUM(irq_num))) * 4))
#define INTC_MIRn_CLR_OFF(irq_num)  (0x88 + ((8*(INTC_IRQ_REG_NUM(irq_num))) * 4))
#define INTC_MIRn_SET_OFF(irq_num)  (0x8c + ((8*(INTC_IRQ_REG_NUM(irq_num))) * 4))

#define NUM_INTERRUPTS      128
/* Interrupt numbers */
/*Only defining potentialy useful once for us*/
#define EMUINT_INT_NUM      0
#define COMMTX_INT_NUM      1
#define COMMRX_INT_NUM      2
#define BENCH_INT_NUM       3
#define ELM_IRQ_INT_NUM     4
#define SSM_WFI_IRQ_INT_NUM 5
#define SSM_IRQ_INT_NUM     6
#define NMI_INT_NUM         7
#define SEC_EVNT_INT_NUM    8
#define L3DEBUG_INT_NUM     9
#define L3APPINT_INT_NUM    10
#define PRCMINT_INT_NUM     11
#define WDT0_INT_NUM        15
#define USBSS_INT_NUM       17
#define USBINT0_INT_NUM     18
#define USBINT1_INT_NUM     19
#define MMCSD1_INT_NUM      28
#define MMCSD2_INT_NUM      29
#define I2C2_INT_NUM        30
#define GPIOINT2A_INT_NUM   32
#define GPIOINT2B_INT_NUM   33
#define UART3_INT_NUM       44
#define UART4_INT_NUM       45
#define UART5_INT_NUM       46
#define GPIOINT3A_INT_NUM   62
#define GPIOINT3B_INT_NUM   63
#define MMCSD0_INT_NUM      64
#define TINT0_INT_NUM       66
#define TINT1_1MS_INT_NUM   67
#define TINT2_INT_NUM       68
#define TINT3_INT_NUM       69
#define I2C0_INT_NUM        70
#define I2C1_INT_NUM        71
#define UART0_INT_NUM       72
#define UART1_INT_NUM       73
#define UART2_INT_NUM       74
#define MAILBOX0_INT_NUM    77
#define WDT1_INT_NUM        91
#define TINT4_INT_NUM       92
#define TINT5_INT_NUM       93
#define TINT6_INT_NUM       94
#define TINT7_INT_NUM       95
#define GPIOINT0A_INT_NUM   96
#define GPIOINT0B_INT_NUM   97
#define GPIOINT1A_INT_NUM   98
#define GPIOINT1B_INT_NUM   99

/* INT numbers*/
#define UART0_INT_NUM 72

void INTC_init();
bool INTC_register_irq(uint32_t int_num, void (*handler)(void));
bool INTC_enable_irq(uint32_t int_num);
bool INTC_disable_irq(uint32_t int_num);
bool INTC_clear_irq(uint32_t int_num);

void CPU_irq_enable(void);
void CPU_irq_disable(void);

void setup_vector_table(void);

/**
 * @brief Does all the necessary setup for the interrupts
 *        calls the functions to setup the vector table 
 *        and the interrupt controller and enables cpu interrupts
 */
void system_interrupt_init(void);

static inline void INTC_set_priority(uint32_t int_num, uint32_t priority) {
    /* Always route through IRQ (bit 0 set to 0)*/
    /* bits 2-7 mask the priority*/
    REG32_write(INTC_BASE_ADDR, INTC_ILEn_OFF(int_num), (priority<<2)&0b111111100);
}

static inline uint16_t INTC_active_irq_num(void) {
    return (uint16_t)(REG32_read(INTC_BASE_ADDR, INTC_SIR_IRQ_OFF) & 0x7F);
}

void dump_undef_info(uint32_t cpsr, uint32_t fault_addr);

 #endif /* __INTERRUPT_H    */
