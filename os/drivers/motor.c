//  Motor control driver

#include <motor.h>
#include <gpio.h>

motor_t motors[NUM_MOTORS];

/*
 * Reads the current state of the motor
 * 0 = stopped, 1 = moving forward, -1 = moving backwards
 */
int motor_read(int motor_num, void* buf, int count) {
	motor_t motor;
	int v = *((int*) buf);
	int pin1_value, pin2_value;

	if (motor_num < 0 || motor_num > NUM_MOTORS)
		return -1;

	motor = motors[motor_num];
	pin1_value = GPIO_read(motor.gpio_base, motor.pin1);
	pin2_value = GPIO_read(motor.gpio_base, motor.pin2);
	if ((pin1_value == 0 && pin2_value == 0) || (pin1_value == 1 && pin2_value == 1))
		v = 0;	
	if ((pin1_value == 0 && pin2_value == 1))
		v = 1;
	if ((pin1_value == 1 && pin2_value == 0))
		v = -1;
	
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
	/*
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
	*/
}


