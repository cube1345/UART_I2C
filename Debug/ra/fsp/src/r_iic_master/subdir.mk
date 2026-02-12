################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ra/fsp/src/r_iic_master/r_iic_master.c 

C_DEPS += \
./ra/fsp/src/r_iic_master/r_iic_master.d 

OBJS += \
./ra/fsp/src/r_iic_master/r_iic_master.o 

SREC += \
UART_I2C.srec 

MAP += \
UART_I2C.map 


# Each subdirectory must supply rules for building sources it contributes
ra/fsp/src/r_iic_master/%.o: ../ra/fsp/src/r_iic_master/%.c
	$(file > $@.in,-mcpu=cortex-m33 -mthumb -mfloat-abi=hard -mfpu=fpv5-sp-d16 -O2 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-strict-aliasing -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal -g -D_RENESAS_RA_ -D_RA_CORE=CM33 -D_RA_ORDINAL=1 -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/src" -I"." -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra/fsp/inc" -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra/fsp/inc/api" -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra/fsp/inc/instances" -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra/arm/CMSIS_6/CMSIS/Core/Include" -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra_gen" -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra_cfg/fsp_cfg/bsp" -I"E:/WorkSpace/RA_WorkSpace/UART_I2C/ra_cfg/fsp_cfg" -std=c99 -Wno-stringop-overflow -Wno-format-truncation --param=min-pagesize=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" -x c "$<")
	@echo Building file: $< && arm-none-eabi-gcc @"$@.in"

