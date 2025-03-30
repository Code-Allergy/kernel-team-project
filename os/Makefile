# OS makefile

# Compiler and assembler flags
ASFLAGS = -mcpu=cortex-a8 -g

# Directories
UTILS_DIR := $(TOP_DIR)/common
DRIVERS_DIR := $(TOP_DIR)/os/drivers
FS_DIR      := $(TOP_DIR)/os/fs
MMU_DIR     := $(TOP_DIR)/os/mmu
BOOT_DIR    := $(TOP_DIR)/boot
SCHEDULER_DIR := $(TOP_DIR)/os/scheduler


CCDEFINES = -DPLATFORM=$(PLATFORM)

LDSCRIPT = $(OS_DIR)/linker.ld

LDFLAGS = -T $(LDSCRIPT) \
			--build-id=none \
			-nostdlib \
			-static \
			$(GLOBAL_LD_FLAGS)

LDLIBS = $(GLOBAL_LD_LIBS)

IFLAGS = 	-I$(OS_DIR)/ \
			-I$(DRIVERS_DIR) \
			-I$(UTILS_DIR) \
			-I$(INTERRUPTS_DIR) \
			-I$(FS_DIR) \
			-I$(MMU_DIR) \
			-I$(BOOT_DIR) \
			-I$(SCHEDULER_DIR)

OUTPUT_ELF = $(OS_DIR)/build/kernel.elf
OUTPUT_BIN = $(OS_DIR)/build/kernel.bin
# INTERRUPTS_DIR = interrupts
#----------- COMMON
# Utils
UTILS_OBJS := $(patsubst $(UTILS_DIR)/%.c, $(UTILS_DIR)/build/%.o, $(wildcard $(UTILS_DIR)/*.c))


utils: utils_build_dir $(UTILS_OBJS)

utils_build_dir:
	mkdir -p $(UTILS_DIR)/build
$(UTILS_DIR)/build/%.o: $(UTILS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

#----------- DRIVERS
DRIVER_OBJS := $(patsubst $(DRIVERS_DIR)/%.c, $(DRIVERS_DIR)/build/%.o, $(wildcard $(DRIVERS_DIR)/*.c))

drivers: driver_build_dir $(DRIVER_OBJS)

driver_build_dir:
	mkdir -p $(DRIVERS_DIR)/build
$(DRIVERS_DIR)/build/%.o: $(DRIVERS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@


#----------- INTERRUPTS
#INTERRUPTS_OBJS := $(patsubst $(INTERRUPTS_DIR)/%.c, $(INTERRUPTS_DIR)/build/%.o, $(wildcard $(INTERRUPTS_DIR)/*.c))
INTERRUPTS_OBJS := $(patsubst $(INTERRUPTS_DIR)/%.c, $(INTERRUPTS_DIR)/build/%.o, $(wildcard $(INTERRUPTS_DIR)/*.c))
INTERRUPTS_OBJS += $(patsubst $(INTERRUPTS_DIR)/%.S, $(INTERRUPTS_DIR)/build/%.o, $(wildcard $(INTERRUPTS_DIR)/*.S))


interrupts: interrupts_build_dir $(INTERRUPTS_OBJS)

interrupts_build_dir:
	echo $(INTERRUPTS_DIR)
	mkdir -p $(INTERRUPTS_DIR)/build
$(INTERRUPTS_DIR)/build/%.o: $(INTERRUPTS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@
$(INTERRUPTS_DIR)/build/%.o: $(INTERRUPTS_DIR)/%.S
	$(CC) $(IFLAGS) $(CFLAGS) $(ASFLAGS) -c $< -o $@
#----------- FS
FS_OBJS := $(patsubst $(FS_DIR)/%.c, $(FS_DIR)/build/%.o, $(wildcard $(FS_DIR)/*.c))

fs: fs_build_dir $(FS_OBJS)

fs_build_dir:
	mkdir -p $(FS_DIR)/build
$(FS_DIR)/build/%.o: $(FS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@


#----------- MMU
MMU_OBJS := $(patsubst $(MMU_DIR)/%.c, $(MMU_DIR)/build/%.o, $(wildcard $(MMU_DIR)/*.c))

mmu: mmu_build_dir $(MMU_OBJS)

mmu_build_dir:
	mkdir -p $(MMU_DIR)/build

$(MMU_DIR)/build/%.o: $(MMU_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@


#----------- SCHEDULER
SCHEDULER_OBJS := $(patsubst $(SCHEDULER_DIR)/%.c, $(SCHEDULER_DIR)/build/%.o, $(wildcard $(SCHEDULER_DIR)/*.c))
SCHEDULER_OBJS += $(patsubst $(SCHEDULER_DIR)/%.S, $(SCHEDULER_DIR)/build/%.o, $(wildcard $(SCHEDULER_DIR)/*.S))

scheduler: scheduler_build_dir $(SCHEDULER_OBJS)

scheduler_build_dir:
	mkdir -p $(SCHEDULER_DIR)/build

$(SCHEDULER_DIR)/build/%.o: $(SCHEDULER_DIR)/%.c | scheduler_build_dir
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

$(SCHEDULER_DIR)/build/%.o: $(SCHEDULER_DIR)/%.S | scheduler_build_dir
	$(CC) $(IFLAGS) $(CFLAGS) $(ASFLAGS) -c $< -o $@



#----------- KERNEL
KERNEL_OBJS := $(patsubst $(OS_DIR)/%.c, $(OS_DIR)/build/%.o, $(wildcard $(OS_DIR)/*.c))

kernel: kernel_build_dir $(OUTPUT_BIN) disassemble

kernel_build_dir:
	mkdir -p $(OS_DIR)/build

$(OUTPUT_BIN): $(OUTPUT_ELF)
	$(OBJCOPY) -O binary $< $@

$(OUTPUT_ELF): $(KERNEL_OBJS) $(UTILS_OBJS) $(DRIVER_OBJS) $(INTERRUPTS_OBJS) $(FS_OBJS) $(MMU_OBJS) $(SCHEDULER_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OS_DIR)/build/%.o: $(OS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

disassemble: $(OUTPUT_ELF)
	arm-none-eabi-objdump -d $(OUTPUT_ELF) > $(OS_DIR)/build/kernel_disassembly.txt

#----------- CLEAN
clean:
	rm -f \
	    $(OUTPUT_ELF) $(OUTPUT_BIN) $(OS_DIR)/build/kernel_disassembly.txt \
		$(OS_DIR)/build/*.o $(OS_DIR)/build/*.d \
		$(UTILS_DIR)/build/*.o $(UTILS_DIR)/build/*.d \
		$(DRIVERS_DIR)/build/*.o $(DRIVERS_DIR)/build/*.d \
		$(INTERRUPTS_DIR)/build/*.o $(INTERRUPTS_DIR)/build/*.d \
		$(FS_DIR)/build/*.o $(FS_DIR)/build/*.d \
		$(MMU_DIR)/build/*.o $(MMU_DIR)/build/*.d \
		$(SCHEDULER_DIR)/build/*.o $(SCHEDULER_DIR)/build/*.d
