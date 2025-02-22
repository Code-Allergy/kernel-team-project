# OS makefile

# Compiler and assembler flags
ASFLAGS = -mcpu=cortex-a8 -g

# Directories
UTILS_DIR := $(TOP_DIR)/common
DRIVERS_DIR := $(TOP_DIR)/os/drivers

CCDEFINES = -DPLATFORM=$(PLATFORM)

LDFLAGS = -T $(LDSCRIPT) \
			--build-id=none \
			-nostdlib \
			-static

IFLAGS = 	-I$(OS_DIR)/ \
			-I$(DRIVERS_DIR) \
			-I$(UTILS_DIR)

CFLAGS =	-Wall \
			-Wextra \
			-mcpu=cortex-a8  \
			-mfloat-abi=soft \
			-Os \
			-std=gnu90 \
			-static \
			-ffreestanding \
			-fbuiltin \
			-marm \
			-MMD \
			-MP \
			$(CCDEFINES) 

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

#----------- CLEAN
clean:
	rm -f \
		$(UTILS_DIR)/build/*.o $(UTILS_DIR)/build/*.d \
		$(DRIVERS_DIR)/build/*.o $(DRIVERS_DIR)/build/*.d \
