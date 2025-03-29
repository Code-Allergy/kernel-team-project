//  Motor control driver

#include <motor.h>
#include <gpio.h>
#include <driver_defs.h>

#define LEFT 0
#define RIGHT 1
#define LEFT_MOTOR_PIN1  (0x1 << 2)
#define LEFT_MOTOR_PIN2  (0x1 << 3)
#define RIGHT_MOTOR_PIN1 (0x1 << 4)
#define RIGHT_MOTOR_PIN2 (0x1 << 5)
#define LEFT_MOTOR_EN    (0x1 << 6)
#define RIGHT_MOTOR_EN   (0x1 << 7)
motor_t motors[NUM_MOTORS];


void motors_stop();
void motors_start();
void motors_set_dir(int dirs);


void motors_stop() {
	int i;
	for (i = 0; i < NUM_MOTORS; i++) {
		GPIO_clear(motors[i].gpio_base, motors[i].en_pin);
	}
}

void motors_start() {
	int i;
	motor_state_t state;
	for (i = 0; i < NUM_MOTORS; i++) {
		state= motors[i].state;
		switch (state){
			case MOTOR_STOP:
				GPIO_clear(motors[i].gpio_base, motors[i].pin1);
				GPIO_clear(motors[i].gpio_base, motors[i].pin2);
				break;
			case MOTOR_FORWARD:
				GPIO_set(motors[i].gpio_base, motors[i].pin1);
				GPIO_clear(motors[i].gpio_base, motors[i].pin2);
				break;
			case MOTOR_BACKWARD:
				GPIO_clear(motors[i].gpio_base, motors[i].pin1);
				GPIO_set(motors[i].gpio_base, motors[i].pin2);
				break;
			default:
				break;
		}
		if(state != MOTOR_STOP){
			/* Enable the motor */
			GPIO_set(motors[i].gpio_base, motors[i].en_pin);
		}
	}
}

/*
 * Initializes the motor driver
 */
void motor_init(void) {
	motors[LEFT].motor_number = LEFT;
	motors[LEFT].gpio_base = GPIO1_BASE;
	motors[LEFT].pin1 = LEFT_MOTOR_PIN1;
	motors[LEFT].pin2 = LEFT_MOTOR_PIN2;
	motors[LEFT].en_pin = LEFT_MOTOR_EN;

	motors[RIGHT].motor_number = RIGHT;
	motors[RIGHT].gpio_base = GPIO1_BASE;
	motors[RIGHT].pin1 = RIGHT_MOTOR_PIN1;
	motors[RIGHT].pin2 = RIGHT_MOTOR_PIN2;
	motors[RIGHT].en_pin = RIGHT_MOTOR_EN;

	/* Set the pins to output */
	GpioSetPinMode(GPIO1_BASE, LEFT_MOTOR_PIN1 | LEFT_MOTOR_PIN2 | LEFT_MOTOR_EN |
			RIGHT_MOTOR_PIN1 | RIGHT_MOTOR_PIN2 | RIGHT_MOTOR_EN, GpioPinOut);
	GPIO_clear(GPIO1_BASE, LEFT_MOTOR_PIN1 | LEFT_MOTOR_PIN2 | LEFT_MOTOR_EN |
			RIGHT_MOTOR_PIN1 | RIGHT_MOTOR_PIN2 | RIGHT_MOTOR_EN); /* set all to 0 */
}

void motors_set_dir(int dirs){

	if(dirs & MOTOR_DIR_STOP){
		motors[LEFT].state = MOTOR_STOP;
		motors[RIGHT].state = MOTOR_STOP;
		return;
	}

	if(dirs & MOTOR_DIR_FORWARD){
		motors[LEFT].state = MOTOR_FORWARD;
		motors[RIGHT].state = MOTOR_FORWARD;
	}

	if(dirs & MOTOR_DIR_BACKWARD){
		motors[LEFT].state = MOTOR_BACKWARD;
		motors[RIGHT].state = MOTOR_BACKWARD;
	}

	if(dirs & MOTOR_DIR_LEFT){
		motors[LEFT].state = MOTOR_STOP;
	}

	if(dirs & MOTOR_DIR_RIGHT){
		motors[RIGHT].state = MOTOR_STOP;
	}
}



/**
 * @brief Handles motor control operations based on the given command.
 *
 * This function processes motor control commands and performs the appropriate
 * actions, such as setting the motor direction or speed.
 * motors are stopped before executing the command and restarted afterward.
 *
 * @param cmd The command to execute. Supported commands:
 *            - MOTOR_SET_DIR: Set the motor direction.
 *            - MOTOR_SET_SPEED: Set the motor speed (currently unimplemented).
 * @param value The value associated with the command. For example, the direction
 *              or speed value.
 *
 * @return returns 0 on success or error code on failure.
 */
int motor_ioctl(int cmd, int value) {
	int i;
	motors_stop();
	switch (cmd) {
	case MOTOR_SET_DIR:
		motors_set_dir(value);
		break;
	case MOTOR_SET_SPEED:
		/* Unimplemented */
		break;
	default:
		break;
	}
	motors_start();
	return 0;
}


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
	pin1_value = GPIO_get(motor.gpio_base, motor.pin1);
	pin2_value = GPIO_get(motor.gpio_base, motor.pin2);
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
	if (v == 0) {
		GPIO_clear(motor.gpio_base, motor.pin1);
		GPIO_clear(motor.gpio_base, motor.pin2);
		return 0;
	}
	else if (v > 0) {
		GPIO_set(motor.gpio_base, motor.pin1);
		GPIO_clear(motor.gpio_base, motor.pin2);
		return 0;
	}
	else {
		GPIO_clear(motor.gpio_base, motor.pin1);
		GPIO_set(motor.gpio_base, motor.pin2);
		return 0;
	}
}


/*
 * Test sequence for the motor driver
 */
extern void delay(unsigned int secs);
void motor_test_sequence(void) {
	int i;

	motor_init();
	
	while(1){
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_FORWARD);
		delay(5);
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_BACKWARD);
		delay(5);
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_FORWARD | MOTOR_DIR_LEFT);
		delay(5);
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_BACKWARD | MOTOR_DIR_LEFT);
		delay(5);
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_FORWARD | MOTOR_DIR_RIGHT);
		delay(5);
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_BACKWARD | MOTOR_DIR_RIGHT);
		delay(5);
		motor_ioctl(MOTOR_SET_DIR, MOTOR_DIR_STOP);
		delay(5);
	}
}


