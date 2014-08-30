#/*
#    FreeRTOS V8.0.1 - Copyright (C) 2014 Real Time Engineers Ltd.
#	
#
#    ***************************************************************************
#     *                                                                       *
#     *    FreeRTOS tutorial books are available in pdf and paperback.        *
#     *    Complete, revised, and edited pdf reference manuals are also       *
#     *    available.                                                         *
#     *                                                                       *
#     *    Purchasing FreeRTOS documentation will not only help you, by       *
#     *    ensuring you get running as quickly as possible and with an        *
#     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
#     *    the FreeRTOS project to continue with its mission of providing     *
#     *    professional grade, cross platform, de facto standard solutions    *
#     *    for microcontrollers - completely free of charge!                  *
#     *                                                                       *
#     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
#     *                                                                       *
#     *    Thank you for using FreeRTOS, and thank you for your support!      *
#     *                                                                       *
#    ***************************************************************************
#
#
#    This file is part of the FreeRTOS distribution.
#
#    FreeRTOS is free software; you can redistribute it and/or modify it under
#    the terms of the GNU General Public License (version 2) as published by the
#    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
#    >>>NOTE<<< The modification to the GPL is included to allow you to
#    distribute a combined work that includes FreeRTOS without being obliged to
#    provide the source code for proprietary components outside of the FreeRTOS
#    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
#    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
#    more details. You should have received a copy of the GNU General Public
#    License and the FreeRTOS license exception along with FreeRTOS; if not it
#    can be viewed here: http://www.freertos.org/a00114.html and also obtained
#    by writing to Richard Barry, contact details for whom are available on the
#    FreeRTOS WEB site.
#
#    1 tab == 4 spaces!
#
#    http://www.FreeRTOS.org - Documentation, latest information, license and
#    contact details.
#
#    http://www.SafeRTOS.com - A version that is certified for use in safety
#    critical systems.
#
#    http://www.OpenRTOS.com - Commercial support, development, porting,
#    licensing and training services.
#*/


RTOS_SOURCE_DIR=/home/virtual/freertos/FreeRTOS/Source/
DEMO_COMMON_DIR=/home/virtual/freertos/FreeRTOS/Demo/Common/Minimal/
DEMO_INCLUDE_DIR=/home/virtual/freertos/FreeRTOS/Demo/Common/include/

CC?=arm-none-eabi-gcc
OBJCOPY?=arm-none-eabi-objcopy
LDSCRIPT?=lpc2388.ld

LINKER_FLAGS=-mthumb -nostartfiles -Xlinker -oBULLSHIT.elf -Xlinker -M -Xlinker -Map=BULLSHIT.map

DEBUG=-g
OPTIM=-O0


CFLAGS= $(DEBUG) \
		$(OPTIM) \
		-T$(LDSCRIPT) \
		-I . \
		-I ./mmc \
		-I ./usb \
		-I ./dma \
		-I ./uart \
		-I ./debug \
		-I $(RTOS_SOURCE_DIR)/include \
		-I $(RTOS_SOURCE_DIR)/portable/GCC/ARM7_LPC23xx \
		-D ROWLEY_LPC23xx \
		-D THUMB_INTERWORK \
		-mcpu=arm7tdmi \
		-D PACK_STRUCT_END=__attribute\(\(packed\)\) \
		-D ALIGN_STRUCT_END=__attribute\(\(aligned\(4\)\)\) \
		-fomit-frame-pointer \
		-mthumb-interwork \
		-fno-dwarf2-cfi-asm \
		-fno-strict-aliasing
		
THUMB_SOURCE= \
		./main.c \
		./syscalls.c \
		./usb/usb_driver.c \
		./uart/uart_driver.c \
		./debug/printf-stdarg.c \
		./mmc/mmc_driver.c \
		./dma/dma_driver.c \
		$(RTOS_SOURCE_DIR)/portable/GCC/ARM7_LPC23xx/port.c \
		$(RTOS_SOURCE_DIR)/portable/MemMang/heap_2.c \
		$(RTOS_SOURCE_DIR)/queue.c \
		$(RTOS_SOURCE_DIR)/tasks.c \
		$(RTOS_SOURCE_DIR)/list.c

ARM_SOURCE= \
		$(RTOS_SOURCE_DIR)/portable/GCC/ARM7_LPC23xx/portISR.c

THUMB_OBJS = $(THUMB_SOURCE:.c=.o)
ARM_OBJS = $(ARM_SOURCE:.c=.o)


all: BULLSHIT.bin

BULLSHIT.bin : BULLSHIT.hex
	$(OBJCOPY) BULLSHIT.elf -O binary BULLSHIT.bin
	 
BULLSHIT.hex : BULLSHIT.elf
	$(OBJCOPY) BULLSHIT.elf -O ihex BULLSHIT.hex

BULLSHIT.elf : $(THUMB_OBJS) $(ARM_OBJS) boot.s Makefile
	$(CC) $(CFLAGS) $(ARM_OBJS) $(THUMB_OBJS) $(LIBS) boot.s $(LINKER_FLAGS) 

$(THUMB_OBJS) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) -mthumb $< -o $@

$(ARM_OBJS) : %.o : %.c Makefile FreeRTOSConfig.h
	$(CC) -c $(CFLAGS) $< -o $@

clean :
	-rm -f $(THUMB_OBJS)
	-rm -f $(ARM_OBJS)
	touch Makefile
	-rm -f BULLSHIT.elf
	-rm -f BULLSHIT.hex
	
	
	




