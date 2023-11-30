GCCPATH=
BASENAME=arm-none-eabi

PCBNO=
LSINAME=
ROMVER=
ULOADCMD=@echo "Noloading... done!"

TARGPROC=-mthumb-interwork -march=armv5t 
OPTFLAGS=-Og -Wall -Wpedantic -Werror 

EXTRAOBJS=

# Fetch eventual toolchain path and prefix, pcb and processor variant 
-include .private/mytarget
# This file could look like
# GCCPATH="D:/ARM_GCC/6 2017-q2-update/bin/"
# BASENAME=arm-none-eabi
# PCBNO=-DPCB100466824
# ROMVER=-DROM_2_0
# LSINAME=-DLSI_TTB5501D
# ULOADCMD=@scp bin/firmware.bin root@hallonpaj:/mnt/usb/user_code.bin

CC=$(GCCPATH)$(BASENAME)-gcc
CPP=$(GCCPATH)$(BASENAME)-c++
LD=$(GCCPATH)$(BASENAME)-ld
OBJCOPY=$(GCCPATH)$(BASENAME)-objcopy
SIZE=$(GCCPATH)$(BASENAME)-size
GDB=$(GCCPATH)$(BASENAME)-gdb


HEADERPATHS=\
	-Ihw\
	-I.private

SHAREDC=$(OPTFLAGS) -ffunction-sections -nostartfiles -g3 -gdwarf-2 $(HEADERPATHS) $(PCBNO) $(LSINAME) $(ROMVER)

CFLAGS=$(TARGPROC) $(SHAREDC) -std=gnu99
CPPFLAGS=$(TARGPROC) $(SHAREDC) 


.PHONY: all upload clean
all : firmware

upload : all
	$(ULOADCMD)


# Additional objects to build
-include .private/myobjs

ALLOBJS=\
	obj/entry.o \
	obj/main.o \
	obj/interrupts.o \
	$(EXTRAOBJS)

# hackjob...
-include .private/mybuilds

obj/main.o : main.c
	@$(CC) $(CFLAGS) -c $< -o $@

obj/interrupts.o : interrupts.c
	@$(CC) $(CFLAGS) -c $< -o $@

obj/entry.o : entry.s
	@$(CC) $(CFLAGS) -c $< -o $@




# Later linkers may require this to shut up:  --no-warn-rwx-segment 

firmware : $(ALLOBJS)
	@$(LD) -Map=bin/firmware.map --gc-sections -Tlink.ld -o bin/firmware.elf $(ALLOBJS)
	@$(OBJCOPY) -O binary bin/firmware.elf bin/firmware.bin
	@$(OBJCOPY) -O ihex bin/firmware.elf bin/firmware.hex
	@$(SIZE) -B bin/firmware.elf


clean :
	@echo Cleaning..
	@rm -f obj/*.o
	@rm -f bin/*.elf
	@rm -f bin/*.bin
	@rm -f bin/*.hex
