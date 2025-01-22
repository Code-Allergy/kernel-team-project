# Default platform is QEMU
PLATFORM ?=BBB

# Set platform-specific include
ifeq ($(PLATFORM), BBB)
	LDSCRIPT = bbb_memory.ld
else
	LDSCRIPT = qemu_memory.ld
endif

# Compiler and assembler flags
AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a8 -g
LD = arm-none-eabi-ld
LDFLAGS = -T $(LDSCRIPT) -Wl --build-id=none -nostdlib -static -nostartfiles
OBJCOPY = arm-none-eabi-objcopy
IFLAGS = -I. -I./src -I./src/bootloader
CC = arm-none-eabi-gcc
CCDEFINES = -DPLATFORM=$(PLATFORM)
CFLAGS = -Wall -Wextra -g -O0 -mcpu=cortex-a8  -mfloat-abi=soft  $(CCDEFINES)
CFLAGS += -static -ffreestanding -fbuiltin -marm

OUTPUT_ELF = bootloader.elf
OUTPUT_BIN = bootloader.bin
OUTPUT_MLO = bootloader.mlo
MLO_DEST_ADDR = 0x402f0400
OUTPUT_SDIMG = bootloader.img
PREFILES = $(BUILD_DIR)/init.i $(BUILD_DIR)/main.i $(BUILD_DIR)/start.i
#OBJFILES = bootloader.o #uart.o uart_test.o
OBJFILES = $(BUILD_DIR)/init.o $(BUILD_DIR)/main.o $(BUILD_DIR)/start.o

BOOT_DIR = src/bootloader
SRC_DIR = src
BUILD_DIR = build
TOP_DIR = .

all: $(OUTPUT_ELF) $(OUTPUT_BIN) $(OUTPUT_SDIMG) disassemble

# create ELF file
$(OUTPUT_ELF): $(OBJFILES)
	$(LD) -T $(LDSCRIPT) -o $(OUTPUT_ELF) $(OBJFILES)

# create binary file
$(OUTPUT_BIN): $(OUTPUT_ELF)
	$(OBJCOPY) -O binary $(OUTPUT_ELF) $(OUTPUT_BIN)

# assemble the source
$(BUILD_DIR)/init.o: $(BOOT_DIR)/init.S memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -E $(BOOT_DIR)/init.S -o $(BUILD_DIR)/init.i
	$(AS) $(IFLAGS) $(ASFLAGS) $(BUILD_DIR)/init.i -o $(BUILD_DIR)/init.o

$(BUILD_DIR)/main.o: $(BOOT_DIR)/main.S memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -E $(BOOT_DIR)/main.S -o $(BUILD_DIR)/main.i
	$(AS) $(IFLAGS) $(ASFLAGS) $(BUILD_DIR)/main.i -o $(BUILD_DIR)/main.o

$(BUILD_DIR)/start.o: $(BOOT_DIR)/start.S memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -E $(BOOT_DIR)/start.S -o $(BUILD_DIR)/start.i
	$(AS) $(IFLAGS) $(ASFLAGS) $(BUILD_DIR)/start.i -o $(BUILD_DIR)/start.o



uart.o: $(SRC_DIR)/uart.c $(SRC_DIR)/uart.h memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -c $(SRC_DIR)/uart.c

uart_test.o: $(SRC_DIR)/uart_test.c $(SRC_DIR)/uart.h memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -c $(SRC_DIR)/uart_test.c



# clean up generated files
clean:
	rm -f $(OBJFILES) $(PREFILES) $(OUTPUT_ELF) $(OUTPUT_BIN) disassembly.txt

$(OUTPUT_MLO): $(OUTPUT_BIN)
	$(TOP_DIR)/sdimager/mk-gpimage $(MLO_DEST_ADDR) $< $@

$(OUTPUT_SDIMG): $(OUTPUT_MLO)
	cp $(TOP_DIR)/sdimager/raw-mmc-header.img $@
	dd if=$< of=$@ iflag=fullblock conv=sync seek=1 status=none
	echo 'label: dos' | /sbin/sfdisk --quiet $@


flash: $(OUTPUT_SDIMG)
ifndef DEV
	$(error DEV is not set. Run "make flash DEV=path/to/dev" to flash the image)
endif
	$(TOP_DIR)/sdimager/flash_img.sh $(OUTPUT_SDIMG) $(DEV)

disassemble: $(OUTPUT_ELF)
	arm-none-eabi-objdump -d $(OUTPUT_ELF) > disassembly.txt

# Run QEMU with the binary output
qemu-gdb: $(OUTPUT_BIN)
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN) -S -gdb tcp::1234
qemu-run: $(OUTPUT_BIN)
	qemu-system-arm -M cubieboard -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN)
