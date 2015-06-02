################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/ext_convert.c \
../src/ext_mode_client.c \
../src/ext_util.c \
../src/rtiostream_interface.c 

OBJS += \
./src/ext_convert.o \
./src/ext_mode_client.o \
./src/ext_util.o \
./src/rtiostream_interface.o 

C_DEPS += \
./src/ext_convert.d \
./src/ext_mode_client.d \
./src/ext_util.d \
./src/rtiostream_interface.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DEXTMODE_TCPIP_TRANSPORT -DSL_EXT_SO -I"/home/control/workspace/ext_mode_client/incl" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


