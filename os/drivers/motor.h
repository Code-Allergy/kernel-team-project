//  Motor control API
#ifndef MOTOR_H
#define MOTOR_H

#include <gpio.h>

#define NUM_MOTORS 4

/* to go forward, (pin1, pin2) = (0, 1)
 * to go back, (pin1, pin2) = (1, 0)
 * to stop (pin1, pin2) = (0, 0) or (1, 1)
 */
typedef struct {
	int motor_number;
	enum GpioIOBase gpio_base;
	int pin1;
	int pin2;
} motor_t;

#endif //__MOTOR_H__

