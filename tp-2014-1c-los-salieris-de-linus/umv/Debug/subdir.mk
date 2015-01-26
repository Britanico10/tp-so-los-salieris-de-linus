################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../consola.c \
../mensajes.c \
../segmentos.c \
../thread_sockets.c \
../umv.c \
../variables.c 

OBJS += \
./consola.o \
./mensajes.o \
./segmentos.o \
./thread_sockets.o \
./umv.o \
./variables.o 

C_DEPS += \
./consola.d \
./mensajes.d \
./segmentos.d \
./thread_sockets.d \
./umv.d \
./variables.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


