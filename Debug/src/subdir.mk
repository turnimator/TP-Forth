################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/builtins.c \
../src/compiler.c \
../src/custom.c \
../src/dictionary.c \
../src/logg.c \
../src/n_queue.c \
../src/parser.c \
../src/program.c \
../src/runtime.c \
../src/smtok.c \
../src/task.c \
../src/tp_queue.c \
../src/tpforth.c \
../src/variable.c 

C_DEPS += \
./src/builtins.d \
./src/compiler.d \
./src/custom.d \
./src/dictionary.d \
./src/logg.d \
./src/n_queue.d \
./src/parser.d \
./src/program.d \
./src/runtime.d \
./src/smtok.d \
./src/task.d \
./src/tp_queue.d \
./src/tpforth.d \
./src/variable.d 

OBJS += \
./src/builtins.o \
./src/compiler.o \
./src/custom.o \
./src/dictionary.o \
./src/logg.o \
./src/n_queue.o \
./src/parser.o \
./src/program.o \
./src/runtime.o \
./src/smtok.o \
./src/task.o \
./src/tp_queue.o \
./src/tpforth.o \
./src/variable.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/builtins.d ./src/builtins.o ./src/compiler.d ./src/compiler.o ./src/custom.d ./src/custom.o ./src/dictionary.d ./src/dictionary.o ./src/logg.d ./src/logg.o ./src/n_queue.d ./src/n_queue.o ./src/parser.d ./src/parser.o ./src/program.d ./src/program.o ./src/runtime.d ./src/runtime.o ./src/smtok.d ./src/smtok.o ./src/task.d ./src/task.o ./src/tp_queue.d ./src/tp_queue.o ./src/tpforth.d ./src/tpforth.o ./src/variable.d ./src/variable.o

.PHONY: clean-src

