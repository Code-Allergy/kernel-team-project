# Default platform is QEMU
PLATFORM ?=BBB

TOP_DIR 		= .
BOOT_DIR 		= $(TOP_DIR)/boot
DRIVERS_DIR 	= $(OS_DIR)/drivers
BUILD_DIR 		= $(TOP_DIR)/build
OUTPUT_SDIMG 	= $(BUILD_DIR)/sd.img
export OS_DIR 	= $(TOP_DIR)/os
export INTERRUPTS_DIR = $(OS_DIR)/interrupts
export FS_DIR 		  = $(OS_DIR)/fs

export ToolPrefix ?= arm-none-eabi

export AS 		= ${ToolPrefix}-as
export LD 		= ${ToolPrefix}-ld
export OBJCOPY  = ${ToolPrefix}-objcopy
export CC 		= ${ToolPrefix}-gcc

all: boot sdimg

utils:
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		utils

drivers: utils
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		drivers

fs: utils
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		fs

boot: utils drivers fs interrupts
	make -f $(BOOT_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM)
	mkdir -p $(BUILD_DIR)
	cp $(BOOT_DIR)/build/MLO $(BOOT_DIR)/build/boot_disassembly.txt $(BUILD_DIR)/

kernel: | $(BUILD_DIR)
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		kernel
	cp $(OS_DIR)/build/kernel.bin $(OS_DIR)/build/kernel_disassembly.txt $(BUILD_DIR)/

# TODO: This will be compiled with the rest of the OS when interrupts are moved to the OS
interrupts:
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		interrupts

os: utils
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM)

clean:
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		clean
	make -f $(BOOT_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		clean


sdimg: boot kernel
	$(TOP_DIR)/sdimager/mksdimage.sh $(BUILD_DIR)/MLO $(BUILD_DIR)/kernel.bin $(OUTPUT_SDIMG)

flash: sdimg
ifndef DEV
	$(error DEV is not set. Run "make flash DEV=path/to/dev" to flash the image)
endif
	$(TOP_DIR)/sdimager/flash_img.sh $(OUTPUT_SDIMG) $(DEV)

# TODO: Fix these
# Run QEMU with the binary output
qemu-gdb: $(OUTPUT_BIN)
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN) -S -gdb tcp::1234
qemu-run: $(OUTPUT_BIN)
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN)
