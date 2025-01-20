# Default platform is QEMU
PLATFORM ?= QEMU

# Set platform-specific include
ifeq ($(PLATFORM), QEMU)
    MEM_ADDR_INC = mem_addresses_qemu.h
else
    MEM_ADDR_INC = mem_addresses_bbb.h
endif

# Compiler and assembler flags
AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a8 -march=armv7-a -g 
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
IFLAGS = -I. -I./src
CC = arm-none-eabi-gcc
CFLAGS = -Wall -g -O0 -mcpu=cortex-a8 -march=armv7-a

OUTPUT_ELF = bootloader.elf
OUTPUT_BIN = bootloader.bin
OBJFILES = bootloader.o uart.o uart_test.o
# TODO: make conditional
LDSCRIPT = qemu_memory.ld

SRC_DIR = src

all: $(OUTPUT_ELF) $(OUTPUT_BIN)

# create ELF file
$(OUTPUT_ELF): $(OBJFILES)
	$(LD) -T $(LDSCRIPT) -o $(OUTPUT_ELF) $(OBJFILES)

# create binary file
$(OUTPUT_BIN): $(OUTPUT_ELF)
	$(OBJCOPY) -O binary $(OUTPUT_ELF) $(OUTPUT_BIN)

# assemble the source
bootloader.o: $(SRC_DIR)/bootloader.s $(MEM_ADDR_INC)
	$(AS) $(IFLAGS) $(ASFLAGS) -o bootloader.o $(SRC_DIR)/bootloader.s

uart.o: $(SRC_DIR)/uart.c $(SRC_DIR)/uart.h memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -c $(SRC_DIR)/uart.c

uart_test.o: $(SRC_DIR)/uart_test.c $(SRC_DIR)/uart.h memory_map.h
	$(CC) $(IFLAGS) $(CFLAGS) -c $(SRC_DIR)/uart_test.c



# clean up generated files
clean:
	rm -f $(OBJFILES) $(OUTPUT_ELF) $(OUTPUT_BIN)

disassemble: $(OUTPUT_ELF)
	arm-none-eabi-objdump -d $(OUTPUT_ELF) > disassembly.txt

# Run QEMU with the binary output
qemu-gdb: $(OUTPUT_BIN)
	qemu-system-arm -M versatilepb -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN) -S -gdb tcp::1234
qemu-run: $(OUTPUT_BIN)
	qemu-system-arm -M versatilepb -cpu cortex-a8 -nographic -kernel $(OUTPUT_BIN)
