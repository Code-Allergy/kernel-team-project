#include <interrupt.h>
#include <types.h>
#include <utils.h>
#include <uart.h>
#include <gpio.h>

const unsigned int VECTOR_TABLE_DEST_ADDR = 0x4030FC00;

void (*intc_vector_table[NUM_INTERRUPTS])(void);

static void default_handler(void)
{
    /*do nothing*/
    return;
}

void INTC_init()
{
    uint16_t i;

    /*Reset Arm INTC*/
    REG32_write(INTC_BASE_ADDR, INTC_SYSCONFIG_OFF, 0x2);
    while ((REG32_read(INTC_BASE_ADDR, INTC_STATUS_OFF) & 0x1) != 0x1)
        ;

    /*INTC clock free-run mode*/
    REG32_write_masked(INTC_BASE_ADDR, INTC_SYSCONFIG_OFF, 0x1, 0x1);
    /*INTC funtional clock free-run and input sync clock free-run*/
    REG32_write_masked(INTC_BASE_ADDR, INTC_IDLE_OFF, 0x3, 0x01);
    /*Set the priority threshold*/
    /*0xFF disables the priority threshold*/
    REG32_write(INTC_BASE_ADDR, INTC_THRESHOLD_OFF, 0xFF);

    /*Default handlers*/
    for (i = 0; i < NUM_INTERRUPTS; i++)
    {
        intc_vector_table[i] = default_handler;
    }
}

bool INTC_register_irq(uint32_t int_num, void (*handler)(void))
{
    if (int_num >= NUM_INTERRUPTS)
    {
        return false;
    }
    intc_vector_table[int_num] = handler;
    return true;
}

bool INTC_enable_irq(uint32_t int_num)
{
    if (int_num >= NUM_INTERRUPTS)
    {
        return false;
    }
    __asm(" dsb");
    REG32_write_masked(INTC_BASE_ADDR,
                       INTC_MIRn_CLR_OFF(int_num),
                       INTC_IRQ_REG_BIT(int_num),
                       INTC_IRQ_REG_BIT(int_num));
    return true;
}

bool INTC_disable_irq(uint32_t int_num)
{
    if (int_num >= NUM_INTERRUPTS)
    {
        return false;
    }
    __asm(" dsb");
    REG32_write_masked(INTC_BASE_ADDR,
                       INTC_MIRn_SET_OFF(int_num),
                       INTC_IRQ_REG_BIT(int_num),
                       INTC_IRQ_REG_BIT(int_num));
    return true;
}

void CPU_irq_enable(void)
{
    asm("    dsb    \n\t"
        "    mrs     r0, CPSR\n\t"
        "    bic     r0, #0x80\n\t"
        "    msr     CPSR, r0");
}

void CPU_irq_disable(void)
{
    asm("    dsb    \n\t"
        "    mrs     r0, CPSR\n\t"
        "    orr     r0, #0x80\n\t"
        "    msr     CPSR, r0");
}

void system_interrupt_init(void)
{
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

#define INTC_SIR_IRQ 0x48200040
#define INTC_BASE    0x48200000

#define DMTIMER_IRQSTATUS_OFFSET 0x28
#define DMTIMER_IRQ_OVERFLOW     0x2 // Match/overflow interrupt
#define DMTIMER2_BASE            0x48040000

uint32_t get_active_irq()
{
    return *((volatile uint32_t*) (INTC_BASE + 0x40)) & 0x7F;
}

void irq_handler(void)
{
    static int irq_count = 0;
    irq_count++;

    uint32_t irq_num;
    uint32_t saved_cpsr;
    volatile uint32_t* irq_sp;
    volatile uint32_t* user_sp;
    uint32_t user_lr;
    int i;

    irq_num = get_active_irq();

    // Only do full debug printout on second IRQ (user mode expected)
    if (irq_count == 2)
    {
        asm volatile("mov %0, sp" : "=r"(irq_sp));
        asm volatile("mrs %0, cpsr" : "=r"(saved_cpsr));

        uint32_t sys_cpsr = (saved_cpsr & ~0x1F) | 0x1F;
        asm volatile("msr cpsr_c, %0" ::"r"(sys_cpsr));
        asm volatile("mov %0, sp" : "=r"(user_sp));
        asm volatile("mov %0, lr" : "=r"(user_lr));
        asm volatile("msr cpsr_c, %0" ::"r"(saved_cpsr));

        uart_printf(">>> IRQ fired: IRQ%d <<<\n", irq_num);

        uart_puts("Top 10 IRQ stack words:\n");
        for (i = 0; i < 10; i++)
        {
            uart_printf("  word: 0x%x\n", irq_sp[i]);
        }

        uart_puts("Top 10 USER stack words:\n");
        for (i = 0; i < 10; i++)
        {
            uart_printf("  word: 0x%x\n", user_sp[i]);
        }

        uart_printf("User LR = 0x%x\n", user_lr);
    }
}

/* This is the same way the default dead loops are set up*/
static unsigned int const vector_table[] = {
    0xE59FF018, /* Opcode for loading PC with the contents of [PC + 0x18] */
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    0xE59FF018,
    (unsigned int) reset_handler_asm,
    (unsigned int) mmu_undef_handler_asm,
    (unsigned int) svc_handler_asm,
    (unsigned int) prefetch_abort_handler_asm,
    (unsigned int) data_abort_handler_asm,
    (unsigned int) reserved_handler_asm,
    (unsigned int) irq_handler_asm,
    (unsigned int) fiq_handler_asm};

extern void set_vector_table_base_addr_asm(unsigned int addr);

void setup_vector_table(void)
{
    uint16_t i;
    set_vector_table_base_addr_asm(VECTOR_TABLE_DEST_ADDR);
    for (i = 0; i < sizeof(vector_table) / sizeof(vector_table[0]); i++)
    {
        ((unsigned int*) VECTOR_TABLE_DEST_ADDR)[i] = vector_table[i];
    }
}

void undef_handler(void) { panic("Undefined instruction exception"); }

void prefetch_abort_handler(void)
{
    uint32_t fault_address, ifsr, spsr, lr;
    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12;
    uint32_t sp, pc;

    // Faulting address from Instruction Fault Address Register (IFAR)
    __asm__ volatile("MRC p15, 0, %0, c6, c0, 2" : "=r"(fault_address));

    // Instruction Fault Status Register
    __asm__ volatile("MRC p15, 0, %0, c5, c0, 1" : "=r"(ifsr));

    // Get SPSR (state the CPU was in before abort)
    __asm__ volatile("MRS %0, SPSR" : "=r"(spsr));

    // Get LR (return address to instruction that caused the abort)
    __asm__ volatile("MOV %0, LR" : "=r"(lr));

    // Print out information about the fault
    uart_printf("PANIC: Prefetch abort exception\n");
    uart_printf("  Fault address (IFAR):  0x%x\n", fault_address);
    uart_printf("  IFSR:                  0x%x\n", ifsr);
    uart_printf("  SPSR:                  0x%x\n", spsr);
    uart_printf("  LR:                    0x%x\n", lr);

    uart_printf("CPU Registers at the time of the exception:\n");
    __asm__ volatile("MOV %0, r0" : "=r"(r0));
    uart_printf("r0:  0x%x\n", r0);

    __asm__ volatile("MOV %0, r1" : "=r"(r1));
    uart_printf("r1:  0x%x\n", r1);

    __asm__ volatile("MOV %0, r2" : "=r"(r2));
    uart_printf("r2:  0x%x\n", r2);

    __asm__ volatile("MOV %0, r3" : "=r"(r3));
    uart_printf("r3:  0x%x\n", r3);

    __asm__ volatile("MOV %0, r4" : "=r"(r4));
    uart_printf("r4:  0x%x\n", r4);

    __asm__ volatile("MOV %0, r5" : "=r"(r5));
    uart_printf("r5:  0x%x\n", r5);

    __asm__ volatile("MOV %0, r6" : "=r"(r6));
    uart_printf("r6:  0x%x\n", r6);

    __asm__ volatile("MOV %0, r7" : "=r"(r7));
    uart_printf("r7:  0x%x\n", r7);

    __asm__ volatile("MOV %0, r8" : "=r"(r8));
    uart_printf("r8:  0x%x\n", r8);

    __asm__ volatile("MOV %0, r9" : "=r"(r9));
    uart_printf("r9:  0x%x\n", r9);

    __asm__ volatile("MOV %0, r10" : "=r"(r10));
    uart_printf("r10: 0x%x\n", r10);

    __asm__ volatile("MOV %0, r11" : "=r"(r11));
    uart_printf("r11: 0x%x\n", r11);

    __asm__ volatile("MOV %0, r12" : "=r"(r12));
    uart_printf("r12: 0x%x\n", r12);

    __asm__ volatile("MOV %0, sp" : "=r"(sp));
    uart_printf("sp:  0x%x\n", sp);

    __asm__ volatile("MOV %0, pc" : "=r"(pc));
    uart_printf("pc:  0x%x\n", pc);

    // Halt the system
    while (1)
        ;
}

void dump_undef_info(uint32_t cpsr, uint32_t fault_addr)
{
    int i;
    uart_puts("We made it to the handler and are now looking at undef \n");

    uart_printf("Checking memory around crash:\n");
    for (i = -4; i <= 4; i++)
    {
        uint32_t* addr = (uint32_t*) (0x9fdffc10 + i * 4);
        uart_printf("0x%x = 0x%x\n", (uint32_t) addr, *addr);
    }

    uint32_t instr = *((volatile uint32_t*) fault_addr);
    uart_printf("UNDEF: At 0x%x, instruction = 0x%x, CPSR = 0x%x\n",
                fault_addr,
                instr,
                cpsr);
    undef_handler();
}

void data_abort_handler(void)
{
    uint32_t fault_address, dfsr, instr, cpsr, lr;

    // Read DFAR: faulting virtual address
    __asm__ volatile("MRC p15, 0, %0, c6, c0, 0" : "=r"(fault_address));

    // Read DFSR: fault type (domain + status)
    __asm__ volatile("MRC p15, 0, %0, c5, c0, 0" : "=r"(dfsr));

    // Read LR and CPSR at time of exception
    __asm__ volatile("MOV %0, lr" : "=r"(lr));
    __asm__ volatile("MRS %0, cpsr" : "=r"(cpsr));

    // Read instruction at LR (should be where fault occurred)
    instr = *((volatile uint32_t*) lr);

    uart_puts("PANIC: Data abort exception!\n");
    uart_printf("  Fault address (DFAR):  0x%x\n", fault_address);
    uart_printf("  Fault status  (DFSR):  0x%x\n", dfsr);
    uart_printf("  Instruction at LR:     0x%x\n", instr);
    uart_printf("  LR (return addr):      0x%x\n", lr);
    uart_printf("  CPSR:                  0x%x\n", cpsr);

    panic("Data abort exception triggered.");
}
