################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: mock\ cpu

dependents:
	-cd /home/utnso/workspace/parser/Debug && $(MAKE) all

# Tool invocations
mock\ cpu: $(OBJS) $(USER_OBJS) /home/utnso/workspace/parser/Debug/libparser.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/workspace/parser/Debug" -o "mock cpu" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) "mock cpu"
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/parser/Debug/libparser.so:

-include ../makefile.targets
