################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/scheduler/scheduler.c 

OBJS += \
./Drivers/scheduler/scheduler.o 

C_DEPS += \
./Drivers/scheduler/scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/scheduler/%.o Drivers/scheduler/%.su Drivers/scheduler/%.cyclo: ../Drivers/scheduler/%.c Drivers/scheduler/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/STworkspace/FOTA_UART1/Drivers/scheduler" -I"D:/STworkspace/FOTA_UART1/Drivers/UART" -I"D:/STworkspace/FOTA_UART1/Drivers/Flash" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-scheduler

clean-Drivers-2f-scheduler:
	-$(RM) ./Drivers/scheduler/scheduler.cyclo ./Drivers/scheduler/scheduler.d ./Drivers/scheduler/scheduler.o ./Drivers/scheduler/scheduler.su

.PHONY: clean-Drivers-2f-scheduler

