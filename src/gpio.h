#ifndef GPIO_H
#define GPIO_H

#include <memory_map.h>
void dumb_delay(void);
void GPIO_init(void); // Only configures GPIO1 for now
void GPIO_set(unsigned int gpio_base, unsigned int pins);
void GPIO_clear(unsigned int gpio_base, unsigned int pins);
#endif