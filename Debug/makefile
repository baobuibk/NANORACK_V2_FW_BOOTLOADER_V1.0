################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/scheduler/subdir.mk
-include Drivers/UART/subdir.mk
-include Drivers/STM32F4xx_HAL_Driver/Src/subdir.mk
-include Drivers/Flash/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := NANORACK_V2_FW_BOOTLOADER_V1.0
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
NANORACK_V2_FW_BOOTLOADER_V1.0.elf \

MAP_FILES += \
NANORACK_V2_FW_BOOTLOADER_V1.0.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
NANORACK_V2_FW_BOOTLOADER_V1.0.list \

OBJCOPY_HEX += \
NANORACK_V2_FW_BOOTLOADER_V1.0.hex \

OBJCOPY_BIN += \
NANORACK_V2_FW_BOOTLOADER_V1.0.bin \


# All Target
all: main-build

# Main-build Target
main-build: NANORACK_V2_FW_BOOTLOADER_V1.0.elf secondary-outputs

# Tool invocations
NANORACK_V2_FW_BOOTLOADER_V1.0.elf NANORACK_V2_FW_BOOTLOADER_V1.0.map: $(OBJS) $(USER_OBJS) D:\final_code\FOTA_DEMO\STM32F407VGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "NANORACK_V2_FW_BOOTLOADER_V1.0.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m4 -T"D:\final_code\FOTA_DEMO\STM32F407VGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="NANORACK_V2_FW_BOOTLOADER_V1.0.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

NANORACK_V2_FW_BOOTLOADER_V1.0.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "NANORACK_V2_FW_BOOTLOADER_V1.0.list"
	@echo 'Finished building: $@'
	@echo ' '

NANORACK_V2_FW_BOOTLOADER_V1.0.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "NANORACK_V2_FW_BOOTLOADER_V1.0.hex"
	@echo 'Finished building: $@'
	@echo ' '

NANORACK_V2_FW_BOOTLOADER_V1.0.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "NANORACK_V2_FW_BOOTLOADER_V1.0.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) NANORACK_V2_FW_BOOTLOADER_V1.0.bin NANORACK_V2_FW_BOOTLOADER_V1.0.elf NANORACK_V2_FW_BOOTLOADER_V1.0.hex NANORACK_V2_FW_BOOTLOADER_V1.0.list NANORACK_V2_FW_BOOTLOADER_V1.0.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
