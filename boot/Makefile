# boot Makefile
# Default platform is BBB
PLATFORM ?=BBB

BOOT_DIR = $(TOP_DIR)/boot
DRIVERS_DIR = $(TOP_DIR)/os/drivers


# Set platform-specific include
ifeq ($(PLATFORM), BBB)
    LDSCRIPT = $(BOOT_DIR)/bbb_memory.ld
else
    LDSCRIPT = qemu_memory.ld
endif

# Compiler and assembler flags
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

ASFLAGS = -mcpu=cortex-a8 -g

CCDEFINES = -DPLATFORM=$(PLATFORM)

LDFLAGS = 	-T $(LDSCRIPT) \
			--build-id=none \
			-nostdlib \
			-static \
			-L$(DRIVERS_DIR) \
			-L$(TOP_DIR)/common

IFLAGS = 	-I$(BOOT_DIR) \
			-I$(TOP_DIR)/os/drivers

CFLAGS = 	-Wall \
			-Wextra \
			-g \
			-O0 \
			-mcpu=cortex-a8  \
			-mfloat-abi=soft \
			-pedantic \
			-static \
			-ffreestanding \
			-fbuiltin \
			-marm \
			-MMD \
			-MP \
			$(CCDEFINES) 

OUTPUT_ELF = $(BOOT_DIR)/bootloader.elf
OUTPUT_BIN = $(BOOT_DIR)/bootloader.bin
OUTPUT_MLO = $(BOOT_DIR)/MLO
MLO_DEST_ADDR = 0x402f0400


# Find all .S and .c files in the current directory
S_FILES := $(wildcard $(BOOT_DIR)/*.S)
C_FILES := $(wildcard $(BOOT_DIR)/*.c)

# Convert them to .o files
OBJFILES := $(S_FILES:.S=.o) $(C_FILES:.c=.o)


all: $(OUTPUT_ELF) $(OUTPUT_BIN) $(OUTPUT_MLO) disassemble

# create ELF file
$(OUTPUT_ELF): $(OBJFILES)
	$(LD) $(LDFLAGS) -o $(OUTPUT_ELF) $(OBJFILES) -luart -lgpio -lutils


# create binary file
$(OUTPUT_BIN): $(OUTPUT_ELF)
	$(OBJCOPY) -O binary $(OUTPUT_ELF) $(OUTPUT_BIN)

$(OUTPUT_MLO): $(OUTPUT_BIN)
	$(TOP_DIR)/sdimager/mk-gpimage $(MLO_DEST_ADDR) $< $@

disassemble: $(OUTPUT_ELF)
	arm-none-eabi-objdump -d $(OUTPUT_ELF) > $(BOOT_DIR)/boot_disassembly.txt

# preprocess and build all .S files
$(BOOT_DIR)/%.o: $(BOOT_DIR)/%.S
	$(CC) $(IFLAGS) $(CFLAGS) -E $< -o $<.i
	$(AS) $(IFLAGS) $(ASFLAGS) $<.i -o $@

# compile all c sources
$(BOOT_DIR)/%.o: $(BOOT_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

# clean up generated files
clean:
	rm -f \
		$(BOOT_DIR)/*.i $(BOOT_DIR)/*.o $(BOOT_DIR)/*.d $(BOOT_DIR)/*.a \
		$(OUTPUT_ELF) \
		$(OUTPUT_BIN) \
		$(BOOT_DIR)/boot_disassembly.txt \
