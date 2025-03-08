# boot Makefile
# Default platform is BBB
PLATFORM ?=BBB

BOOT_DIR = $(TOP_DIR)/boot
DRIVERS_DIR = $(TOP_DIR)/os/drivers
UTILS_DIR = $(TOP_DIR)/common


# Set platform-specific include
ifeq ($(PLATFORM), BBB)
    LDSCRIPT = $(BOOT_DIR)/bbb_memory.ld
else
    LDSCRIPT = qemu_memory.ld
endif

# Compiler and assembler flags
LIBGCC = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
LIBGCC_DIR = $(dir $(LIBGCC))
ASFLAGS = -mcpu=cortex-a8 -g

CCDEFINES = -DPLATFORM=$(PLATFORM)

LDFLAGS = 	-T $(LDSCRIPT) \
			--build-id=none \
			-nostdlib \
			-static \
			-L$(DRIVERS_DIR) \
			-L$(TOP_DIR)/common \
			-L$(LIBGCC_DIR)

LDLIBS = -lgcc

IFLAGS = 	-I$(BOOT_DIR) \
			-I$(TOP_DIR)/os/drivers \
			-I$(TOP_DIR)/common \
			-I$(INTERRUPTS_DIR)

CFLAGS = 	-Wall \
			-Wextra \
			-mcpu=cortex-a8  \
			-mfloat-abi=soft \
			-static \
			-std=gnu90 \
			-pedantic \
			-ffreestanding \
			-fbuiltin \
			-marm \
			-MMD \
			-MP \
			$(CCDEFINES)

OUTPUT_ELF = $(BOOT_DIR)/build/bootloader.elf
OUTPUT_BIN = $(BOOT_DIR)/build/bootloader.bin
OUTPUT_MLO = $(BOOT_DIR)/build/MLO
MLO_DEST_ADDR = 0x402f0400


# Find all .S and .c files in the current directory
S_FILES := $(wildcard $(BOOT_DIR)/*.S)
C_FILES := $(wildcard $(BOOT_DIR)/*.c)

# Convert them to .o files
# TODO: interrupt.o will be moved to the OS alongside the vectors in the future
OBJFILES := $(patsubst $(BOOT_DIR)/%.S,$(BOOT_DIR)/build/%.o,$(S_FILES)) \
			$(patsubst $(BOOT_DIR)/%.c,$(BOOT_DIR)/build/%.o,$(C_FILES)) \
			$(DRIVERS_DIR)/build/uart.o \
			$(DRIVERS_DIR)/build/gpio.o \
			$(UTILS_DIR)/build/utils.o \
			$(UTILS_DIR)/build/circular_buffer.o \
			$(INTERRUPTS_DIR)/build/interrupt.o \
			$(INTERRUPTS_DIR)/build/interrupt_handlers.o \
			$(FS_DIR)/build/fat32.o


all: boot_build_dir $(OUTPUT_ELF) $(OUTPUT_BIN) $(OUTPUT_MLO) disassemble

boot_build_dir:
	mkdir -p $(BOOT_DIR)/build

# Link to ELF
$(OUTPUT_ELF): $(OBJFILES)
	$(LD) $(LDFLAGS) -o $(OUTPUT_ELF) $(OBJFILES) -Map $(BOOT_DIR)/build/bootloader.map $(LDLIBS)

# create binary file
$(OUTPUT_BIN): $(OUTPUT_ELF)
	$(OBJCOPY) -O binary $(OUTPUT_ELF) $(OUTPUT_BIN)

$(OUTPUT_MLO): $(OUTPUT_BIN)
	$(TOP_DIR)/sdimager/mk-gpimage $(MLO_DEST_ADDR) $< $@

disassemble: $(OUTPUT_ELF)
	arm-none-eabi-objdump -d $(OUTPUT_ELF) > $(BOOT_DIR)/build/boot_disassembly.txt

# preprocess and build all .S files
$(BOOT_DIR)/build/%.o: $(BOOT_DIR)/%.S
	$(CC) $(IFLAGS) $(CFLAGS) -E $< -o $@.i
	$(AS) $(IFLAGS) $(ASFLAGS) $@.i -o $@

# compile all c sources
$(BOOT_DIR)/build/%.o: $(BOOT_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

# clean up generated files
clean:
	rm -f \
		$(BOOT_DIR)/build/*.i $(BOOT_DIR)/build/*.o $(BOOT_DIR)/build/*.d $(BOOT_DIR)/build/*.a \
		$(BOOT_DIR)/build/*.map \
		$(OUTPUT_ELF) \
		$(OUTPUT_BIN) \
		$(OUTPUT_MLO) \
		$(BOOT_DIR)/build/boot_disassembly.txt \
