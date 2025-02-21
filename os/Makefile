# OS makefile

# Compiler and assembler flags
ASFLAGS = -mcpu=cortex-a8 -g

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
			-mcpu=cortex-a8  \
			-mfloat-abi=soft \
			-Os \
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
