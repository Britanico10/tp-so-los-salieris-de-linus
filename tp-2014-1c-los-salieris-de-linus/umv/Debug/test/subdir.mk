################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../test/consola.c \
../test/mensajes.c \
../test/menu.c \
../test/segmentos.c \
../test/test_cpu_thread.c \
../test/test_kernel_thread.c \
../test/test_servidor_thread.c \
../test/thread_sockets.c 

OBJS += \
./test/consola.o \
./test/mensajes.o \
./test/menu.o \
./test/segmentos.o \
./test/test_cpu_thread.o \
./test/test_kernel_thread.o \
./test/test_servidor_thread.o \
./test/thread_sockets.o 

C_DEPS += \
./test/consola.d \
./test/mensajes.d \
./test/menu.d \
./test/segmentos.d \
./test/test_cpu_thread.d \
./test/test_kernel_thread.d \
./test/test_servidor_thread.d \
./test/thread_sockets.d 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


