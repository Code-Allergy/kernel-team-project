# Default platform is QEMU
PLATFORM ?=BBB

export TOP_DIR 			= .
export BOOT_DIR 		= $(TOP_DIR)/boot
export OS_DIR 			= $(TOP_DIR)/os
export DRIVERS_DIR 		= $(OS_DIR)/drivers
BUILD_DIR 				= $(TOP_DIR)/build

export ToolPrefix ?= arm-none-eabi

export AS 		= ${ToolPrefix}-as
export LD 		= ${ToolPrefix}-ld
export OBJCOPY  = ${ToolPrefix}-objcopy
export CC 		= ${ToolPrefix}-gcc

all: boot

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

# $(OUTPUT_SDIMG): $(OUTPUT_MLO)
# 	cp $(TOP_DIR)/sdimager/raw-mmc-header.img $@
# 	dd if=$< of=$@ iflag=fullblock conv=sync seek=1 status=none
# 	echo 'label: dos' | /sbin/sfdisk --quiet $@


# flash: $(OUTPUT_SDIMG)
# ifndef DEV
# 	$(error DEV is not set. Run "make flash DEV=path/to/dev" to flash the image)
# endif
# 	$(TOP_DIR)/sdimager/flash_img.sh $(OUTPUT_SDIMG) $(DEV)

# disassemble: $(OUTPUT_ELF)
# 	arm-none-eabi-objdump -d $(OUTPUT_ELF) > disassembly.txt

# TODO: Fix these
# Run QEMU with the binary output
qemu-gdb: $(OUTPUT_BIN)
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN) -S -gdb tcp::1234
qemu-run: $(OUTPUT_BIN)
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN)
