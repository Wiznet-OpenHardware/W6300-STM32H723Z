################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include io6Library/Internet/httpServer/subdir.mk
-include io6Library/Internet/TFTP/subdir.mk
-include io6Library/Internet/SNTP/subdir.mk
-include io6Library/Internet/SNMP/subdir.mk
-include io6Library/Internet/MQTT/MQTTPacket/src/subdir.mk
-include io6Library/Internet/MQTT/subdir.mk
-include io6Library/Internet/FTPServer/subdir.mk
-include io6Library/Internet/FTPClient/subdir.mk
-include io6Library/Internet/DNS/subdir.mk
-include io6Library/Internet/DHCP6/subdir.mk
-include io6Library/Internet/DHCP/subdir.mk
-include io6Library/Internet/AAC/subdir.mk
-include io6Library/Ethernet/W6300/subdir.mk
-include io6Library/Ethernet/W6100/subdir.mk
-include io6Library/Ethernet/W5500/subdir.mk
-include io6Library/Ethernet/W5300/subdir.mk
-include io6Library/Ethernet/W5200/subdir.mk
-include io6Library/Ethernet/W5100S/subdir.mk
-include io6Library/Ethernet/W5100/subdir.mk
-include io6Library/Ethernet/subdir.mk
-include io6Library/Application/multicast/subdir.mk
-include io6Library/Application/loopback/subdir.mk
-include TestProgram/ES_TEST/subdir.mk
-include TestProgram/subdir.mk
-include Drivers/STM32H7xx_HAL_Driver/Src/subdir.mk
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


BUILD_ARTIFACT_NAME := W6300_QSPI_TEST
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
W6300_QSPI_TEST.elf \

MAP_FILES += \
W6300_QSPI_TEST.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
W6300_QSPI_TEST.list \

OBJCOPY_HEX += \
W6300_QSPI_TEST.hex \

OBJCOPY_BIN += \
W6300_QSPI_TEST.bin \


# All Target
all: main-build

# Main-build Target
main-build: W6300_QSPI_TEST.elf secondary-outputs

# Tool invocations
W6300_QSPI_TEST.elf W6300_QSPI_TEST.map: $(OBJS) $(USER_OBJS) C:\projects\W6300-STM32H723Z\W6300_QSPI_Test\STM32H723ZGTX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "W6300_QSPI_TEST.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m7 -T"C:\projects\W6300-STM32H723Z\W6300_QSPI_Test\STM32H723ZGTX_FLASH.ld" --specs=nosys.specs -Wl,-Map="W6300_QSPI_TEST.map" -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

W6300_QSPI_TEST.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "W6300_QSPI_TEST.list"
	@echo 'Finished building: $@'
	@echo ' '

W6300_QSPI_TEST.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "W6300_QSPI_TEST.hex"
	@echo 'Finished building: $@'
	@echo ' '

W6300_QSPI_TEST.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "W6300_QSPI_TEST.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) W6300_QSPI_TEST.bin W6300_QSPI_TEST.elf W6300_QSPI_TEST.hex W6300_QSPI_TEST.list W6300_QSPI_TEST.map default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
