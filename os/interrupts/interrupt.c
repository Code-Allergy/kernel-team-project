#include <interrupt.h>
#include <types.h>
#include <utils.h>
#include <uart.h>
#include <gpio.h>

const unsigned int VECTOR_TABLE_DEST_ADDR = 0x4030FC00;

void (*intc_vector_table[NUM_INTERRUPTS])(void);

static void default_handler(void){
    /*do nothing*/
    return;
}

void INTC_init() {
    uint16_t i;

    /*Reset Arm INTC*/
    REG32_write(INTC_BASE_ADDR, INTC_SYSCONFIG_OFF, 0x2);
    while ((REG32_read(INTC_BASE_ADDR, INTC_STATUS_OFF) & 0x1) != 0x1);

    /*INTC clock free-run mode*/
    REG32_write_masked(INTC_BASE_ADDR, INTC_SYSCONFIG_OFF, 0x1, 0x1);
    /*INTC funtional clock free-run and input sync clock free-run*/
    REG32_write_masked(INTC_BASE_ADDR, INTC_IDLE_OFF, 0x3, 0x01);
    /*Set the priority threshold*/
    /*0xFF disables the priority threshold*/
    REG32_write(INTC_BASE_ADDR, INTC_THRESHOLD_OFF, 0xFF);

    /*Default handlers*/
    for (i = 0; i < NUM_INTERRUPTS; i++) {
        intc_vector_table[i] = default_handler;
    }
}

bool INTC_register_irq(uint32_t int_num, void (*handler)(void)) {
    if (int_num >= NUM_INTERRUPTS) {
        return false;
    }
    intc_vector_table[int_num] = handler;
    return true;
}

bool INTC_enable_irq(uint32_t int_num) {
    if (int_num >= NUM_INTERRUPTS) {
        return false;
    }
    __asm(" dsb");
    REG32_write_masked( INTC_BASE_ADDR,
                        INTC_MIRn_CLR_OFF(int_num),
                        INTC_IRQ_REG_BIT(int_num),
                        INTC_IRQ_REG_BIT(int_num)
                      );
    return true;
}

bool INTC_disable_irq(uint32_t int_num) {
    if (int_num >= NUM_INTERRUPTS) {
        return false;
    }
    __asm(" dsb");
    REG32_write_masked( INTC_BASE_ADDR,
                        INTC_MIRn_SET_OFF(int_num),
                        INTC_IRQ_REG_BIT(int_num),
                        INTC_IRQ_REG_BIT(int_num)
                      );
    return true;
}



void CPU_irq_enable(void) {
    asm("    dsb    \n\t"
        "    mrs     r0, CPSR\n\t"
        "    bic     r0, #0x80\n\t"
        "    msr     CPSR, r0");
}

void CPU_irq_disable(void) {
    asm("    dsb    \n\t"
        "    mrs     r0, CPSR\n\t"
        "    orr     r0, #0x80\n\t"
        "    msr     CPSR, r0");
}

void system_interrupt_init(void) {
    setup_vector_table();
    CPU_irq_enable();
    INTC_init();
}


extern void reset_handler_asm(void);
extern void undef_handler_asm(void);
extern void svc_handler_asm(void);
extern void prefetch_abort_handler_asm(void);
extern void data_abort_handler_asm(void);
extern void irq_handler_asm(void);
extern void fiq_handler_asm(void);
extern void reserved_handler_asm(void);
extern void mmu_undef_handler_asm(void);




#define INTC_SIR_IRQ    0x48200040
#define INTC_BASE       0x48200000

#define DMTIMER_IRQSTATUS_OFFSET 0x28
#define DMTIMER_IRQ_OVERFLOW 0x2  // Match/overflow interrupt
#define DMTIMER2_BASE 0x48040000



uint32_t get_active_irq() {
    return *((volatile uint32_t *)(INTC_BASE + 0x40)) & 0x7F;
}

void irq_handler(void) {
    uint32_t irq_num = get_active_irq();
    *((volatile uint32_t *)(DMTIMER2_BASE + DMTIMER_IRQSTATUS_OFFSET)) = DMTIMER_IRQ_OVERFLOW;
    uart_printf(">>> IRQ fired: IRQ%d <<<\n", irq_num);
}




/* This is the same way the default dead loops are set up*/
static unsigned int const vector_table[] = {
    0xE59FF018,    /* Opcode for loading PC with the contents of [PC + 0x18] */
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    (unsigned int)reset_handler_asm,
    (unsigned int)mmu_undef_handler_asm,
    (unsigned int)svc_handler_asm,
    (unsigned int)prefetch_abort_handler_asm,
    (unsigned int)data_abort_handler_asm,
    (unsigned int)reserved_handler_asm,
    (unsigned int)irq_handler_asm,
    (unsigned int)fiq_handler_asm
};

extern void set_vector_table_base_addr_asm(unsigned int addr);

void setup_vector_table(void) {
    uint16_t i;
    set_vector_table_base_addr_asm(VECTOR_TABLE_DEST_ADDR);
    for (i = 0; i < sizeof(vector_table) / sizeof(vector_table[0]); i++) {
        ((unsigned int*)VECTOR_TABLE_DEST_ADDR)[i] = vector_table[i];
    }
}

void undef_handler(void) {
    panic("Undefined instruction exception");
}

void prefetch_abort_handler(void) {
    uint32_t fault_address, ifsr, spsr, lr;

    // Faulting address from Instruction Fault Address Register (IFAR)
    __asm__ volatile ("MRC p15, 0, %0, c6, c0, 2" : "=r"(fault_address));

    // Instruction Fault Status Register
    __asm__ volatile ("MRC p15, 0, %0, c5, c0, 1" : "=r"(ifsr));

    // Get SPSR (state the CPU was in before abort)
    __asm__ volatile ("MRS %0, SPSR" : "=r"(spsr));

    // Get LR (return address to instruction that caused the abort)
    __asm__ volatile ("MOV %0, LR" : "=r"(lr));

    uart_printf("PANIC: Prefetch abort exception\n");
    uart_printf("  Fault address (IFAR):  0x%x\n", fault_address);
    uart_printf("  IFSR:                  0x%x\n", ifsr);
    uart_printf("  SPSR:                  0x%x\n", spsr);
    uart_printf("  LR:                    0x%x\n", lr);

    while (1); // Halt
}



void dump_undef_info(uint32_t cpsr, uint32_t fault_addr) {
    int i;
    uart_puts("We made it to the handler and are now looking at undef \n");

    uart_printf("Checking memory around crash:\n");
    for (i = -4; i <= 4; i++) {
        uint32_t *addr = (uint32_t *)(0x9fdffc10 + i * 4);
        uart_printf("0x%x = 0x%x\n", (uint32_t)addr, *addr);
    }

    uint32_t instr = *((volatile uint32_t *)fault_addr);
    uart_printf("UNDEF: At 0x%x, instruction = 0x%x, CPSR = 0x%x\n", fault_addr, instr, cpsr);
    undef_handler();   
}


void data_abort_handler(void) {
    uint32_t fault_address, dfsr;
    /* READ DFAR */
    __asm__ volatile ("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_address));
    /* READ DFSR */
    __asm__ volatile ("MRC p15, 0, %0, c5, c0, 0" : "=r" (dfsr));
    panic("Data abort exception at address 0x%x, DFSR: %x\n", fault_address, dfsr);
}
