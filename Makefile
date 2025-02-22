# Default platform is QEMU
PLATFORM ?=BBB

TOP_DIR 		= .
BOOT_DIR 		= $(TOP_DIR)/boot
OS_DIR 			= $(TOP_DIR)/os
DRIVERS_DIR 	= $(OS_DIR)/drivers
BUILD_DIR 		= $(TOP_DIR)/build
OUTPUT_SDIMG 	= $(BUILD_DIR)/sd.img


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

boot: utils drivers
	make -f $(BOOT_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM)
	cp $(BOOT_DIR)/build/MLO $(BOOT_DIR)/build/boot_disassembly.txt $(BUILD_DIR)/

clean:
	make -f $(OS_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		clean
	make -f $(BOOT_DIR)/Makefile \
		TOP_DIR=$(TOP_DIR) \
		PLATFORM=$(PLATFORM) \
		clean


sdimg: boot
	$(TOP_DIR)/sdimager/mksdimage.sh $(BUILD_DIR)/MLO $(OUTPUT_SDIMG)

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
