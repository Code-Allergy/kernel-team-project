#include <memory_map.h>

.cpu	cortex-a8
.text

.global	main
main:
	push	{ lr }

	bl gpio_test
	pop	{ pc }