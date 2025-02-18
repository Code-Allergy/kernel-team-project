# OS makefile

# Directories
DRIVERS_DIR := $(TOP_DIR)/os/drivers

# Compiler and assembler flags
AS = arm-none-eabi-as
ASFLAGS = -mcpu=cortex-a8 -g
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
CC = arm-none-eabi-gcc

# Directories
UTILS_DIR := $(TOP_DIR)/common

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

#----------- COMMON
# Utils
UTILS_OBJS := $(patsubst $(UTILS_DIR)/%.c, $(UTILS_DIR)/%.o, $(wildcard $(UTILS_DIR)/*.c))
UTILS_LIB := $(UTILS_DIR)/libutils.a

utils: $(UTILS_LIB)

$(UTILS_DIR)/%.o: $(UTILS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

$(UTILS_LIB): $(UTILS_OBJS)
	ar rcs $@ $<


#----------- DRIVERS
DRIVER_OBJS := $(patsubst $(DRIVERS_DIR)/%.c, $(DRIVERS_DIR)/%.o, $(wildcard $(DRIVERS_DIR)/*.c))
DRIVER_LIBS := $(patsubst $(DRIVERS_DIR)/%.o, $(DRIVERS_DIR)/lib%.a, $(DRIVER_OBJS))

drivers: $(DRIVER_LIBS)

$(DRIVERS_DIR)/%.o: $(DRIVERS_DIR)/%.c
	$(CC) $(IFLAGS) $(CFLAGS) -c $< -o $@

$(DRIVERS_DIR)/lib%.a: $(DRIVERS_DIR)/%.o
	ar rcs $@ $<

#----------- CLEAN
clean:
	rm -f \
		$(UTILS_DIR)/*.o $(UTILS_DIR)/*.d $(UTILS_DIR)/*.a \
		$(DRIVERS_DIR)/*.o $(DRIVERS_DIR)/*.d $(DRIVERS_DIR)/*.a \
