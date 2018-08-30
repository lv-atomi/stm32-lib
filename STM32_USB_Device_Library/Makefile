# Compile the STM32F4xx USB Device Library

ifdef DEBUG
 TARGET_OPTS = -O0 -g3 -MMD -D DDEBUG
else
 TARGET_OPTS = -O2 -finline -finline-functions-called-once
endif

TARGET_ARCH = -mcpu=cortex-m4 -mthumb
WARNINGS = -Wall -W -Wshadow -Wwrite-strings -Winline
INCLUDE_DIRS = -I Core/inc -I Class/audio/inc -I Class/msc/inc -I Class/cdc/inc -I Class/hid/inc -I Class/dfu/inc -I ../CMSIS_CM4/Include -I ../STM32F4xx_StdPeriph_Driver -I ../STM32F4xx_StdPeriph_Driver/inc
F_NO_STRICT_ALIASING = -fno-strict-aliasing

COMPILE_OPTS = $(WARNINGS) $(TARGET_OPTS) $(MESSAGES) $(F_NO_STRICT_ALIASING) $(INCLUDE_DIRS) $(DEFINES)

ifndef TRGT
TRGT = /usr/bin/
endif

CC = $(TRGT)arm-none-eabi-gcc
CFLAGS = $(COMPILE_OPTS)

AS = $(CC) -x assembler-with-cpp -c $(TARGET_ARCH)
ASFLAGS = $(COMPILE_OPTS)

AR = $(TRGT)arm-none-eabi-ar
ARFLAGS = cr

LIB_OUT = libstm32usb.a

LIB_OBJS = $(sort \
 $(patsubst %.c,%.o,$(shell find . -name "*.c")))


.PHONY: all
all: $(LIB_OUT)

$(LIB_OUT): $(LIB_OBJS)
	echo $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIB_OBJS)

$(LIB_OBJS): $(shell find . -name *.h)

.PHONY: clean
clean:
	-rm -f $(LIB_OBJS) $(LIB_OUT)
