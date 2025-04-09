//  Motor control API
#ifndef MOTOR_H
#define MOTOR_H

#include <gpio.h>
#include <types.h>

#define NUM_MOTORS 2  /*There are 4 physical motors but only 
2 logical motors (left and right)	*/

typedef enum {
	MOTOR_STOP = 0,
	MOTOR_FORWARD,
	MOTOR_BACKWARD,
} motor_state_t;

/* to go forward, (pin1, pin2) = (0, 1)
 * to go back, (pin1, pin2) = (1, 0)
 * to stop (pin1, pin2) = (0, 0) or (1, 1)
 */
typedef struct {
	int motor_number;
	enum GpioIOBase gpio_base;
	int pin1;
	int pin2;
	int en_pin;
	motor_state_t state;
} motor_t;


void motor_init(void);
int motor_read(int motor_num, void* buf, int count);
int motor_write(int motor_num, void* buf, int count);
int motor_ioctl(int cmd, int value);

void motor_test_sequence(void);

#endif //__MOTOR_H__

