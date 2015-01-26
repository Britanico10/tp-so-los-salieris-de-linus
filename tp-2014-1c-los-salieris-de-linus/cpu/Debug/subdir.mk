################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cpu.c \
../memoria.c \
../mensajes.c \
../primitivas.c \
../variables.c 

OBJS += \
./cpu.o \
./memoria.o \
./mensajes.o \
./primitivas.o \
./variables.o 

C_DEPS += \
./cpu.d \
./memoria.d \
./mensajes.d \
./primitivas.d \
./variables.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


