# PREREQUISITES:
# 1. All tools should be run in a Linux/UNIX environment.
# 2. Install a cross compiler for x86. Follow instructions at https://wiki.osdev.org/GCC_Cross-Compiler 
# 	 and change CC path below to point to your compiled binary.
# 3. Download nasm (or another assembler of your choice supporting x86 with Intel syntax).
#    It is recommended to build from source (https://github.com/netwide-assembler/nasm/) 
#    to ensure compatibility with your system's binaries. Add nasm to your system's PATH variable. 

# file paths
SRC_PATH 		:= ./src
BUILD_PATH 		:= ./build

# tools
AS 				:= nasm 
ASFLAGS			:= -f elf
# change the path below to point to your own cross compiler build
# (see  for help)
CC 				:= ~/applications/cross_compiler/bin/i686-elf-gcc 
CFLAGS 			+= -ffreestanding -mno-red-zone -O0 
LD_FLAGS 		:= -Ttext 0x7C00 -nostartfiles -nostdlib
OBJCOPY_FLAGS 	:= -O binary --pad-to 0x10000

# C object files, but isr.c requires special flag; handle separately
C_OBJS 		:= $(patsubst $(SRC_PATH)/%.c,$(BUILD_PATH)/%.o,$(wildcard $(SRC_PATH)/*.c))
C_OBJS 		:= $(filter-out $(BUILD_PATH)/isr.o, $(C_OBJS))

S_OBJS 		:= $(patsubst $(SRC_PATH)/%.s,$(BUILD_PATH)/%.o,$(wildcard $(SRC_PATH)/*.s))
ASM_OBJS	:= $(patsubst $(SRC_PATH)/%.asm,$(BUILD_PATH)/%.o,$(wildcard $(SRC_PATH)/*.asm))


# Binary kernel image
$(BUILD_PATH)/kernel.bin: $(C_OBJS) $(ASM_OBJS) $(BUILD_PATH)/isr.o
	$(CC) -o $(BUILD_PATH)/kernel.elf $(CFLAGS) $(LD_FLAGS) $(wildcard $(BUILD_PATH)/*.o)
	objcopy $(OBJCOPY_FLAGS) $(BUILD_PATH)/kernel.elf $(BUILD_PATH)/kernel.bin
	rm $(BUILD_PATH)/kernel.elf


$(BUILD_PATH)/isr.o: $(SRC_PATH)/isr.c
	$(CC) -c $(CFLAGS) -mgeneral-regs-only $(CPPFLAGS) $< -o $@


# C files
$(C_OBJS): $(BUILD_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@


# Assembly files
$(ASM_OBJS): $(BUILD_PATH)/%.o: $(SRC_PATH)/%.asm
$(S_OBJS): $(BUILD_PATH)/%.o: $(SRC_PATH)/%.s
$(ASM_OBJS) $(S_OBJS):
	$(AS) $(ASFLAGS) $< -o $@


.PHONY: clean
clean:
	rm -f $(C_OBJS) $(ASM_OBJS) $(S_OBJS) $(BUILD_PATH)/kernel.bin $(BUILD_PATH)/kernel.elf
	
