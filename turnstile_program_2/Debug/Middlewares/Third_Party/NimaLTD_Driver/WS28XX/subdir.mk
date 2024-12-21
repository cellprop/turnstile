################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.c 

OBJS += \
./Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.o 

C_DEPS += \
./Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/Third_Party/NimaLTD_Driver/WS28XX/%.o Middlewares/Third_Party/NimaLTD_Driver/WS28XX/%.su Middlewares/Third_Party/NimaLTD_Driver/WS28XX/%.cyclo: ../Middlewares/Third_Party/NimaLTD_Driver/WS28XX/%.c Middlewares/Third_Party/NimaLTD_Driver/WS28XX/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I../I-CUBE-WS28XX -I../Middlewares/Third_Party/NimaLTD_Driver/WS28XX -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-Third_Party-2f-NimaLTD_Driver-2f-WS28XX

clean-Middlewares-2f-Third_Party-2f-NimaLTD_Driver-2f-WS28XX:
	-$(RM) ./Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.cyclo ./Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.d ./Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.o ./Middlewares/Third_Party/NimaLTD_Driver/WS28XX/ws28xx.su

.PHONY: clean-Middlewares-2f-Third_Party-2f-NimaLTD_Driver-2f-WS28XX

