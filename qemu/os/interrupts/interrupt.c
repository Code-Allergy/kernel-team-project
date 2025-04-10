#include <interrupt.h>
const unsigned int VECTOR_TABLE_DEST_ADDR = 0x4030FC00;
void setup_vector_table(void);

void (*intc_vector_table[NUM_INTERRUPTS])(void);

static inline void CPU_irq_disable(void);
static inline void CPU_irq_enable(void);

/* TODO */
static void INTC_init(void)
{
    /* uint32_t vbar_addr = (uint32_t)_vectors; */
    /* if(vbar_addr & 0x1F) { */
    /*     printk("VBAR unaligned! Fix %p → %p\n", */
    /*            vbar_addr, vbar_addr & ~0x1F); */
    /*     vbar_addr &= ~0x1F; */
    /*     panic("VBAR unaligned"); */
    /* } */

    /* setup CSPR for IRQ mode */
    __asm__ volatile("mrs r0, cpsr \n"
                     "bic r0, r0, #0x80 \n"
                     "msr cpsr_c, r0 \n");
}

extern void reset_handler_asm(void);
extern void undef_handler_asm(void);
extern void svc_handler_asm(void);
extern void prefetch_abort_handler_asm(void);
extern void data_abort_handler_asm(void);
extern void irq_handler_asm(void);
extern void fiq_handler_asm(void);
extern void reserved_handler_asm(void);

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
    (unsigned int) undef_handler_asm,
    (unsigned int) svc_handler_asm,
    (unsigned int) prefetch_abort_handler_asm,
    (unsigned int) data_abort_handler_asm,
    (unsigned int) reserved_handler_asm,
    (unsigned int) irq_handler_asm,
    (unsigned int) fiq_handler_asm};

void handle_irq_c(uint32_t process_stack)
{
    uint32_t pending, irq;
    int reg;

    for (reg = 0; reg < 3; reg++)
    {
        pending = INTC->IRQ_PEND[reg];
        while (pending)
        {
            irq = 32 * reg + __builtin_ctz(pending);
            if (intc_vector_table[irq])
            {
                intc_vector_table[irq]();
            }
            pending &= pending - 1;
        }
    }
}

void system_interrupt_init(void)
{
    setup_vector_table();
    CPU_irq_enable();
    INTC_init();
}

extern set_vector_table_base_addr_asm(unsigned int addr);

void setup_vector_table(void)
{
    uint16_t i;
    set_vector_table_base_addr_asm(VECTOR_TABLE_DEST_ADDR);
    for (i = 0; i < sizeof(vector_table) / sizeof(vector_table[0]); i++)
    {
        ((unsigned int*) VECTOR_TABLE_DEST_ADDR)[i] = vector_table[i];
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
    INTC_IRQ_ENABLE(int_num);
    return true;
}

static inline void CPU_irq_enable(void)
{
    asm("    dsb    \n\t"
        "    mrs     r0, CPSR\n\t"
        "    bic     r0, #0x80\n\t"
        "    msr     CPSR, r0");
}

static inline void CPU_irq_disable(void)
{
    asm("    dsb    \n\t"
        "    mrs     r0, CPSR\n\t"
        "    orr     r0, #0x80\n\t"
        "    msr     CPSR, r0");
}