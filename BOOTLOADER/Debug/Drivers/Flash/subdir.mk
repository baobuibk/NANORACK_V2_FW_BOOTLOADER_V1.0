################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Flash/flash.c 

OBJS += \
./Drivers/Flash/flash.o 

C_DEPS += \
./Drivers/Flash/flash.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Flash/%.o Drivers/Flash/%.su Drivers/Flash/%.cyclo: ../Drivers/Flash/%.c Drivers/Flash/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/STworkspace/FOTA_UART1/Drivers/scheduler" -I"D:/STworkspace/FOTA_UART1/Drivers/UART" -I"D:/STworkspace/FOTA_UART1/Drivers/Flash" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Flash

clean-Drivers-2f-Flash:
	-$(RM) ./Drivers/Flash/flash.cyclo ./Drivers/Flash/flash.d ./Drivers/Flash/flash.o ./Drivers/Flash/flash.su

.PHONY: clean-Drivers-2f-Flash

