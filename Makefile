.DEFAULT_GOAL := all

LOCAL_TOOLCHAIN := /opt/st/stm32cubeide_2.2.0/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.linux64_1.0.100.202602081740/tools/bin
TOOLCHAIN ?= $(if $(wildcard $(LOCAL_TOOLCHAIN)/arm-none-eabi-gcc),$(LOCAL_TOOLCHAIN),)
CROSS_COMPILE := $(if $(TOOLCHAIN),$(TOOLCHAIN)/,)arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
SIZE := $(CROSS_COMPILE)size
OBJCOPY := $(CROSS_COMPILE)objcopy
SOLENOID_HOLD_ENABLED ?= 0
HOST_CC ?= cc
BUILD ?= $(if $(filter 1,$(SOLENOID_HOLD_ENABLED)),build/hold,build)

CPPFLAGS := -DSTM32C031xx -DNUCLEO_C031C6 -DSOLENOID_HOLD_ENABLED=$(SOLENOID_HOLD_ENABLED) -Icourse/chipheaders/Device -Icourse/chipheaders/Include -Icourse/mylib -ILibrary -ISrc
CFLAGS := -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft -std=gnu11 -Og -g3 -Wall -Wextra -MMD -MP -ffunction-sections -fdata-sections
LDFLAGS := -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft -TSTM32C031C6TX_FLASH.ld --specs=nano.specs --specs=nosys.specs -Wl,--gc-sections

COMMON := Src/syscalls.c Src/sysmem.c Library/sysinit.c Library/uart.c Library/uart1.c
OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(COMMON)) $(BUILD)/Startup/startup_stm32c031c6tx.o
PROTOTYPE := $(OBJECTS) $(BUILD)/Library/ADC.o $(BUILD)/Library/ble.o $(BUILD)/Src/SafeLogic.o

-include $(OBJECTS:.o=.d) $(PROTOTYPE:.o=.d) $(BUILD)/Src/Lab9Prototype.d $(BUILD)/Src/Passthrough.d

all: $(BUILD)/Lab9Prototype.bin
hold:
	$(MAKE) SOLENOID_HOLD_ENABLED=1 all
passthrough: $(BUILD)/Passthrough.bin
test: $(BUILD)/test_safe_logic
	$(BUILD)/test_safe_logic

$(BUILD)/test_safe_logic: Src/SafeLogic.c Src/SafeLogic.h tests/test_safe_logic.c
	@mkdir -p $(dir $@)
	$(HOST_CC) -std=c11 -Wall -Wextra -Werror -ISrc Src/SafeLogic.c tests/test_safe_logic.c -o $@

$(BUILD)/Lab9Prototype.elf: $(BUILD)/Src/Lab9Prototype.o $(PROTOTYPE) STM32C031C6TX_FLASH.ld
	$(CC) $(LDFLAGS) -Wl,-Map=$(@:.elf=.map) $(filter %.o,$^) -Wl,--start-group -lc -lm -Wl,--end-group -o $@
	$(SIZE) $@

$(BUILD)/Passthrough.elf: $(BUILD)/Src/Passthrough.o $(OBJECTS) STM32C031C6TX_FLASH.ld
	$(CC) $(LDFLAGS) -Wl,-Map=$(@:.elf=.map) $(filter %.o,$^) -Wl,--start-group -lc -lm -Wl,--end-group -o $@
	$(SIZE) $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: all hold passthrough test
