#ifndef __TIMER_H__
#define __TIMER_H__

#include <types.h>

#define TIMER_MAX_COUNT 0xFFFFFFFFu

/* Timer indexes*/
#define TIMER2 2
#define TIMER3 3
#define TIMER4 4
#define TIMER5 5
#define TIMER6 6
#define TIMER7 7

#define TIMER0_BASE 0x44E05000
#define TIMER1_BASE 0x44E31000
#define TIMER2_BASE 0x48040000
#define TIMER3_BASE 0x48042000
#define TIMER4_BASE 0x48044000
#define TIMER5_BASE 0x48046000
#define TIMER6_BASE 0x48048000
#define TIMER7_BASE 0x4804A000


/* Register offsets*/
/*
0h TIDR Identification Register Section 20.1.5.1
10h TIOCP_CFG Timer OCP Configuration Register Section 20.1.5.2
20h IRQ_EOI Timer IRQ End-of-Interrupt Register Section 20.1.5.3
24h IRQSTATUS_RAW Timer Status Raw Register Section 20.1.5.4
28h IRQSTATUS Timer Status Register Section 20.1.5.5
2Ch IRQENABLE_SET Timer Interrupt Enable Set Register Section 20.1.5.6
30h IRQENABLE_CLR Timer Interrupt Enable Clear Register Section 20.1.5.7
34h IRQWAKEEN Timer IRQ Wakeup Enable Register Section 20.1.5.8
38h TCLR Timer Control Register Section 20.1.5.9
3Ch TCRR Timer Counter Register Section 20.1.5.10
40h TLDR Timer Load Register Section 20.1.5.11
44h TTGR Timer Trigger Register Section 20.1.5.12
48h TWPS Timer Write Posting Bits Register Section 20.1.5.13
4Ch TMAR Timer Match Register Section 20.1.5.14
50h TCAR1 Timer Capture Register Section 20.1.5.15
54h TSICR Timer Synchronous Interface Control Register Section 20.1.5.16
58h TCAR2 Timer Capture Register Section 20.1.5.17
*/
#define TIDR_OFF            0x00
#define TIOCP_CFG_OFF       0x10
#define IRQ_EOI_OFF         0x20
#define IRQSTATUS_RAW_OFF   0x24
#define IRQSTATUS_OFF       0x28
#define IRQENABLE_SET_OFF   0x2C
#define IRQENABLE_CLR_OFF   0x30
#define IRQWAKEEN_OFF       0x34
#define TCLR_OFF            0x38
#define TCRR_OFF            0x3C
#define TLDR_OFF            0x40
#define TTGR_OFF            0x44
#define TWPS_OFF            0x48
#define TMAR_OFF            0x4C
#define TCAR1_OFF           0x50
#define TSICR_OFF           0x54
#define TCAR2_OFF           0x58

/*
We are only supporting timers 2 to 7 for now as they have the same setup process
and are easier to configure.
All timers are set up in 32-bit mode, periodic mode, with overflow interrupts enabled
for tick generation.
*/

/**
 * @brief Initializes the timer with the specified index and tick interval.
 *
 * This function sets up a timer with the given index and configures it to
 * trigger at intervals specified by `timer_tick_ms`. When the timer ticks,
 * the provided callback function `timer_tick_func` will be called.
 * 
 * NOTE: This function only supports timers 2 to 7 currently and will return
 *        false if passed an index not in that range.
 *
 * @param timer_index The index of the timer to initialize.
 * @param timer_tick_ms The interval in milliseconds at which the timer should tick.
 * @param timer_tick_func The callback function to be called on each timer tick.
 * @return true if the timer was successfully initialized, false otherwise.
 */
bool timer_init(uint16_t timer_index, uint32_t timer_tick_ms, void (*timer_tick_func)(void));

/**
 * @brief Starts the timer with the specified index.
 *
 * This function starts the timer with the given index. The timer will begin
 * counting and will trigger the callback function at the interval specified
 * during initialization.
 * The timer must be initialized with timer_init() before it can be started.
 * 
 * NOTE: This function only supports timers 2 to 7 currently and will return
 *        false if passed an index not in that range.
 *
 * @param timer_index The index of the timer to start.
 * @return true if the timer was successfully started, false otherwise.
 */
bool timer_start(uint16_t timer_index);

/**
 * @brief Stops the timer with the specified index.
 *
 * This function stops the timer with the given index. The timer will stop
 * counting and will no longer trigger the callback function.
 * 
 * NOTE: This function only supports timers 2 to 7 currently and will return
 *        false if passed an index not in that range.
 *
 * @param timer_index The index of the timer to stop.
 * @return true if the timer was successfully stopped, false otherwise.
 */
bool timer_stop(uint16_t timer_index);

uint32_t timer_value(uint16_t timer_index);

#endif /*__TIMER_H__*/