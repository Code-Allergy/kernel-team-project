# Default platform is QEMU
PLATFORM ?=BBB
TOP_DIR 		= .
ifeq ($(PLATFORM),QEMU)
    TOP_DIR = ./qemu
endif

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

# check if mtools is available, otherwise use root script
ifeq (, $(shell which mcopy))
	export MAKE_SDIMG_SCRIPT = sudo $(TOP_DIR)/sdimager/mksdimage.sh
else
	export MAKE_SDIMG_SCRIPT = $(TOP_DIR)/sdimager/mksdimage-rootless.sh
endif

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
	$(MAKE_SDIMG_SCRIPT) $(BUILD_DIR)/MLO $(BUILD_DIR)/kernel.bin $(OUTPUT_SDIMG)

flash: sdimg
ifndef DEV
	$(error DEV is not set. Run "make flash DEV=path/to/dev" to flash the image)
endif
	$(TOP_DIR)/sdimager/flash_img.sh $(OUTPUT_SDIMG) $(DEV)

# TODO: Fix these
# Run QEMU with the binary output
qemu-gdb: $(OUTPUT_SDIMG)
	qemu-img resize $(OUTPUT_SDIMG) 128M
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(BOOT_DIR)/build/bootloader.bin -sd $(OUTPUT_SDIMG) -d guest_errors,unimp,int \
	 -S -gdb tcp::1234
qemu-run: $(OUTPUT_SDIMG)
	qemu-img resize $(OUTPUT_SDIMG) 128M
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(BOOT_DIR)/build/bootloader.bin -sd $(OUTPUT_SDIMG) -d guest_errors,unimp,int -D qemu.log
