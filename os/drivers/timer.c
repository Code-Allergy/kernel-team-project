#include <timer.h>
#include <types.h>
#include <clock_module.h>
#include <interrupt.h>
#include <uart.h>


void handle_timer2_irq();
void handle_timer3_irq();
void handle_timer4_irq();
void handle_timer5_irq();
void handle_timer6_irq();
void handle_timer7_irq();
void handle_timer_irq(uint16_t timer_index);

static const int timer_l4ls_clk_en_bit[8] = {
    -1,
    -1,
    1 << 14,
    1 << 15,
    1 << 16,
    1 << 27,
    1 << 28,
    1 << 13
};

static const uint32_t timer_base[8] = {
    -1,
    -1,
    TIMER2_BASE,
    TIMER3_BASE,
    TIMER4_BASE,
    TIMER5_BASE,
    TIMER6_BASE,
    TIMER7_BASE
};

static const uint32_t timer_module_clk_off[8] = {
    -1,
    -1,
    CM_PER_TIMER2_CLKCTRL,
    CM_PER_TIMER3_CLKCTRL,
    CM_PER_TIMER4_CLKCTRL,
    CM_PER_TIMER5_CLKCTRL,
    CM_PER_TIMER6_CLKCTRL,
    CM_PER_TIMER7_CLKCTRL
};

static const uint32_t timer_pll_clksel_off[8] = {
    -1,
    -1,
    CM_CLKSEL_TIMER2_CLK,
    CM_CLKSEL_TIMER3_CLK,
    CM_CLKSEL_TIMER4_CLK,
    CM_CLKSEL_TIMER5_CLK,
    CM_CLKSEL_TIMER6_CLK,
    CM_CLKSEL_TIMER7_CLK
};

static const int16_t timer_irq_num[8] = {
    130,
    130,
    TINT2_INT_NUM,
    TINT3_INT_NUM,
    TINT4_INT_NUM,
    TINT5_INT_NUM,
    TINT6_INT_NUM,
    TINT7_INT_NUM
};

typedef void (*void_func)(void);
static void_func timer_tick_funcs[8] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
static const void_func timer_irq_handlers[8] = {
    NULL,
    NULL,
    handle_timer2_irq,
    handle_timer3_irq,
    handle_timer4_irq,
    handle_timer5_irq,
    handle_timer6_irq,
    handle_timer7_irq
};

#define TIMER_REG_WRITE_WAIT_DONE() \
    while(REG32_read_masked(timer_base[timer_index], TWPS_OFF, 0x1F) & 0x1F)

bool timer_init(uint16_t timer_index, 
                uint32_t timer_tick_ms, 
                void (*timer_tick_func)(void)
                ){

    if(timer_index < 2 || timer_index > 7){
        return false;
    }

    uart_puts("Timer init\n");
    uart_puts("DPLL select\n");
    /*CM_DPLL clock select if required. using CLK_32KHZ clk*/
    REG32_write(CM_DPLL_BASE, timer_pll_clksel_off[timer_index], 0x2);
    while((REG32_read(CM_DPLL_BASE, timer_pll_clksel_off[timer_index]) & 0x3)!= 0x2);
    uart_puts("CM_PER clk enable\n");
    /*Clock enable*/
    REG32_write_masked(CM_PER_BASE, timer_module_clk_off[timer_index], 0x3, 0x2);
    while(
      (REG32_read_masked(CM_PER_BASE, timer_module_clk_off[timer_index], 0x3<<16) & 0x3)
    );

    /* OCP interface software reset*/
    uart_puts("OCP reset\n");
    REG32_write_masked(timer_base[timer_index], TIOCP_CFG_OFF, 0x1, 0x1);
    while((REG32_read_masked(timer_base[timer_index], TIOCP_CFG_OFF, 0x1) & 0x1));
    
    uart_puts("Timer disable\n");    
    /*****Timer setup ******/
    /*stop the time*/
    REG32_write_masked(timer_base[timer_index], TCLR_OFF, 0x1, 0x0);
    TIMER_REG_WRITE_WAIT_DONE();
    uart_puts("Timer setup\n");
    /*Timer control setup*/
    REG32_write_masked(timer_base[timer_index], TCLR_OFF, 
        0x7FFE, 
        0x0000
        | (0x1 << 1) /*AR*/
        | (0x0 << 2) /*PTV*/
        | (0x0 << 5) /*PRE*/
        | (0x0 << 6) /*CE*/
        | (0x0 << 7) /*SCPWM*/
        | (0x0 << 8) /*TCM*/
        | (0x0 << 10) /*TRG*/
        | (0x0 << 12) /*PT*/
        | (0x0 << 13) /*CAPT*/
        | (0x0 << 14) /*GPO_CFG*/
    );
    TIMER_REG_WRITE_WAIT_DONE();
    
    uart_puts("load value\n");
    /*Load the timer with the tick value*/
    REG32_write(timer_base[timer_index], TLDR_OFF, timer_tick_ms * 32);
    TIMER_REG_WRITE_WAIT_DONE();
    uart_puts("irq enable\n");
    /*overflow irq en. Disable the other irq types*/
    REG32_write(timer_base[timer_index], IRQENABLE_SET_OFF, 0x2);
    TIMER_REG_WRITE_WAIT_DONE();
    uart_puts("irq register\n");
    timer_tick_funcs[timer_index] = timer_tick_func;
    INTC_register_irq(timer_irq_num[timer_index], timer_irq_handlers[timer_index]);

    return true;
}

bool timer_start(uint16_t timer_index){
    if(timer_index < 2 || timer_index > 7){
        return false;
    }

    /*start the timer*/
    REG32_write_masked(timer_base[timer_index], TCLR_OFF, 0x1, 0x1);
    TIMER_REG_WRITE_WAIT_DONE();
    INTC_enable_irq(timer_irq_num[timer_index]);

    return true;
}

bool timer_stop(uint16_t timer_index){
    if(timer_index < 2 || timer_index > 7){
        return false;
    }

    /*stop the timer*/
    REG32_write_masked(timer_base[timer_index], TCLR_OFF, 0x1, 0x0);
    TIMER_REG_WRITE_WAIT_DONE();
    INTC_disable_irq(timer_irq_num[timer_index]);

    return true;
}

uint32_t timer_value(uint16_t timer_index){
    if(timer_index < 2 || timer_index > 7){
        return 0;
    }

    return REG32_read(timer_base[timer_index], TCRR_OFF);
}

void handle_timer_irq(uint16_t timer_index){
    /*call the timer tick function*/
    if(timer_tick_funcs[timer_index] != NULL){
        timer_tick_funcs[timer_index]();
    }
    /*clear the interrupt*/
    REG32_write_masked(timer_base[timer_index], IRQSTATUS_OFF, 0x1, 0x1);
    TIMER_REG_WRITE_WAIT_DONE();
}

void handle_timer2_irq(){
    uint16_t timer_index = 2;
    handle_timer_irq(timer_index);
}

void handle_timer3_irq(){
    uint16_t timer_index = 3;
    handle_timer_irq(timer_index);
}

void handle_timer4_irq(){
    uint16_t timer_index = 4;
    handle_timer_irq(timer_index);
}

void handle_timer5_irq(){
    uint16_t timer_index = 5;
    handle_timer_irq(timer_index);
}

void handle_timer6_irq(){
    uint16_t timer_index = 6;
    handle_timer_irq(timer_index);
}

void handle_timer7_irq(){
    uint16_t timer_index = 7;
    handle_timer_irq(timer_index);
}