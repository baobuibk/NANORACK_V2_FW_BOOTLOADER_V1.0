################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/UART/RingBuffer.c \
../Drivers/UART/UART.c 

OBJS += \
./Drivers/UART/RingBuffer.o \
./Drivers/UART/UART.o 

C_DEPS += \
./Drivers/UART/RingBuffer.d \
./Drivers/UART/UART.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/UART/%.o Drivers/UART/%.su Drivers/UART/%.cyclo: ../Drivers/UART/%.c Drivers/UART/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/final_code/FOTA_DEMO/Drivers/scheduler" -I"D:/final_code/FOTA_DEMO/Drivers/UART" -I"D:/final_code/FOTA_DEMO/Drivers/Flash" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-UART

clean-Drivers-2f-UART:
	-$(RM) ./Drivers/UART/RingBuffer.cyclo ./Drivers/UART/RingBuffer.d ./Drivers/UART/RingBuffer.o ./Drivers/UART/RingBuffer.su ./Drivers/UART/UART.cyclo ./Drivers/UART/UART.d ./Drivers/UART/UART.o ./Drivers/UART/UART.su

.PHONY: clean-Drivers-2f-UART

