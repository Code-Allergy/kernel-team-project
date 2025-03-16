//  Motor control API
#ifndef MOTOR_H
#define MOTOR_H

#include <gpio.h>

#define NUM_MOTORS 4

typedef struct {
	int motor_number;
	enum GpioIOBase gpio_base;
	int forwards_pin;
	int backwards_pin;
} motor_t;



#endif //__MOTOR_H__

