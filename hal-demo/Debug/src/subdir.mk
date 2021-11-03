################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/freertos_hooks.c \
../src/main.c 

OBJS += \
./src/freertos_hooks.o \
./src/main.o 

C_DEPS += \
./src/freertos_hooks.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -O0 -fmessage-length=0 -ffunction-sections -Werror -Wall -Wextra  -g -Dsdram -Dat91sam9g20 -DTRACE_LEVEL=5 -DDEBUG=1 -D'BASE_REVISION_NUMBER=$(REV)' -D'BASE_REVISION_HASH_SHORT=$(REVHASH_SHORT)' -D'BASE_REVISION_HASH=$(REVHASH)' -I"C:\ISIS\MAAE4997B_CDH\hal-demo\src" -I"C:/ISIS/MAAE4997B_CDH/hal-demo/../hal//freertos/include" -I"C:/ISIS/MAAE4997B_CDH/hal-demo/../hal//hal/include" -I"C:/ISIS/MAAE4997B_CDH/hal-demo/../hal//at91/include" -I"C:/ISIS/MAAE4997B_CDH/hal-demo/../hal//hcc/include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


