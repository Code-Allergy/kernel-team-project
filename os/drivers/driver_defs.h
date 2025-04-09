#ifndef __DRIVER_DEFS_H
#define __DRIVER_DEFS_H

/* For ioctl */
#define MOTOR_SET_DIR     0x1
#define MOTOR_DIR_FORWARD  (0x1 << 0)
#define MOTOR_DIR_BACKWARD (0x1 << 1)
#define MOTOR_DIR_STOP     (0x1 << 2)
#define MOTOR_DIR_LEFT     (0x1 << 3)
#define MOTOR_DIR_RIGHT    (0x1 << 4)

#define MOTOR_SET_SPEED    0x2


#endif  /*__DRIVER_DEFS_H */