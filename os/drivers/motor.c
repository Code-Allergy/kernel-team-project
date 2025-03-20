//  Motor control driver

#include <motor.h>
#include <gpio.h>

motor_t motors[NUM_MOTORS];

/*
 * Reads the current state of the motor
 */
int motor_read(int motor_num, void* buf, int count) {
	motor_t motor;
	v = *((int*) buf);

	if (motor_num < 0 || motor_num > NUM_MOTORS)
		return -1;

	motor = motors[motor_num];
	v = GPIO_read(motor.gpio_base, motor.forwards_pin);
	return 0;
	
}

/*
 * Negative means go backwards. Positive means go forwards
 * 0 means stop
 */
int motor_write(int motor_num, void* buf, int count) {
	motor_t motor;
	int v;

	if (motor_num < 0 || motor_num > NUM_MOTORS)
		return -1;

	motor = motors[motor_num];
	v = *((int*)buf);
	if (v == 0) {
		GPIO_clear(motor.gpio_base, motor.forwards_pin);
		GPIO_clear(motor.gpio_base, motor.backwards_pin);
		return 0;
	}
	else if (v > 0) {
		GPIO_set(motor.gpio_base, motor.forwards_pin);
		return 0;
	}
	else {
		GPIO_set(motor.gpio_base, motor.backwards_pin);
		return 0;
	}
}


