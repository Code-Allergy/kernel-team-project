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

static inline void INTC_set_priority(uint32_t int_num, uint32_t priority) {
    /* Always route through IRQ (bit 0 set to 0)*/
    /* bits 2-7 mask the priority*/
    REG32_write(INTC_BASE_ADDR, INTC_ILEn_OFF(int_num), (priority<<2)&0b111111100);
}

static inline uint16_t INTC_active_irq_num(void) {
    return (uint16_t)(REG32_read(INTC_BASE_ADDR, INTC_SIR_IRQ_OFF) & 0x7F);
}

 #endif /* __INTERRUPT_H    */