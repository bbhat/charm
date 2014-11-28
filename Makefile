###################################################################################
##	
##						Copyright 2009-2013 xxxxxxx, xxxxxxx
##	File:	Makefile
##	Author:	Bala B. (bhat.balasubramanya@gmail.com)
##	Description: Makefile for OS files
##
###################################################################################

ASM		:=	arm-elf-gcc
CC		:=	arm-elf-gcc
LINK	:=	arm-elf-ld
OBJCOPY	:=	arm-elf-objcopy
OBJDUMP	:=	arm-elf-objdump

LIBPATH:=/usr/local/dev-arm/i386-Darwin-arm-gcc-4.6.1/lib/gcc/arm-elf/4.6.1 /opt/local/lib/gcc/arm-elf/4.6.1/fpu /opt/local/arm-elf/lib/fpu

## Initialize default arguments
TARGET		?=	mini210s
DST			?=	build
CONFIG		?=	debug
APP			?=	test_os

## Initialize dependent parameters
ifeq ($(TARGET), tq2440)
	SOC := s3c2440
endif

ifeq ($(TARGET), mini210s)
	SOC := s5pv210
endif

ifeq ($(SOC), s3c2440)
	CORE := arm920t
endif
ifeq ($(SOC), s5pv210)
	CORE := cortex-a8
endif

BUILD_DIR		:=	$(DST)/$(CONFIG)-$(TARGET)
MAP_FILE		:=	$(BUILD_DIR)/$(TARGET).map
BOOT_MAP_FILE	:=	$(BUILD_DIR)/boot.map
LINKERS_SCRIPT	:=	scripts/$(TARGET)/memmap.ld
BOOT_LSCRIPT	:=	scripts/$(TARGET)/boot.ld
DEP_DIR			:=	$(BUILD_DIR)/dep
OBJ_DIR			:=	$(BUILD_DIR)/obj
KERNEL_TARGET	:=	$(BUILD_DIR)/$(TARGET).elf
BOOT_TARGET		:=	$(BUILD_DIR)/boot.elf
RAMDISK_TARGET	:=	$(BUILD_DIR)/ramdisk.img
ROOTFS_PATH		:=	rootfs

## Build list of source and object files
SUBDIRS			:=	sources main

## Include Boot source files
include $(wildcard boot/$(TARGET)/*.mk)

## Include sources from each subdirectories
include $(foreach sdir, $(SUBDIRS), $(wildcard $(sdir)/*.mk))

## Prefix each INCLUDES directory with -I
INCLUDES		:=	$(addprefix -I , $(INCLUDES))

## Create each LIBPATH directory with -L
LIBPATH			:=	$(addprefix -L , $(LIBPATH))

## Build a list of corresponding object files
OBJS		:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(SOURCES))))
BOOT_OBJS	:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(BOOT_SOURCES))))

## Build flags
AFLAGS		:=	-mcpu=$(CORE) -g -mlittle-endian -mfloat-abi=softfp -mfpu=neon
CFLAGS		:=	-Wall -nostdinc -mcpu=$(CORE) -mlittle-endian -mfloat-abi=softfp -mfpu=neon
LDFLAGS		:=	-nostartfiles -nostdlib -T$(LINKERS_SCRIPT) -Map $(MAP_FILE) $(LIBPATH)
ifeq ($(CONFIG),debug)
	CFLAGS	:=	-g -O0 -D DEBUG $(CFLAGS)
	AFLAGS	:=	-g -D DEBUG $(AFLAGS)
else ifeq ($(CONFIG),release)
	CFLAGS	:=	-O2 -D RELEASE $(CFLAGS)
	AFLAGS	:=	-D RELEASE $(AFLAGS)
endif

ifeq ($(TARGET), tq2440)
	CFLAGS := $(CFLAGS) -D TARGET_TQ2440
endif

ifeq ($(TARGET), mini210s)
	CFLAGS := $(CFLAGS) -D TARGET_MINI210S
endif

ifeq ($(SOC), s3c2440)
	CFLAGS := $(CFLAGS) -D SOC_S3C2440
endif
ifeq ($(SOC), s5pv210)
	CFLAGS := $(CFLAGS) -D SOC_S5PV210
endif


## Rule specifications
.PHONY:	all boot dep clean ramdiskmk elfmerge tools kernel usrlib ramdisk mkv210_image write2sd application

all:
	make boot
	make kernel
	make application APP=G2D
	make application APP=SystemMonitor
	make application APP=test_os
	make application APP=srt
	make application APP=test_aperiodic
	make application APP=test_rtc
	make usrlib
	make ramdisk
	
kernel:
	@echo --------------------------------------------------------------------------------
	@echo Building kernel with following parameters:
	@echo --------------------------------------------------------------------------------
	@echo TARGET=$(TARGET) 
	@echo SOC=$(SOC)
	@echo CONFIG=$(CONFIG)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OBJ_DIR=$(OBJ_DIR)
	@echo MAP_FILE=$(MAP_FILE)
	@echo SOURCES=$(SOURCES)
	@echo OBJS=$(OBJS)
	@echo INCLUDES=$(INCLUDES)
	@echo ROOTFS_PATH=$(ROOTFS_PATH)
	@echo
	make $(KERNEL_TARGET)

boot:
	@echo --------------------------------------------------------------------------------
	@echo Building boot with following parameters:
	@echo --------------------------------------------------------------------------------
	@echo TARGET=$(TARGET) 
	@echo SOC=$(SOC)
	@echo CONFIG=$(CONFIG)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OBJ_DIR=$(OBJ_DIR)
	@echo BOOT_SOURCES=$(BOOT_SOURCES)
	@echo BOOT_OBJS=$(BOOT_OBJS)
	@echo
	make $(BOOT_TARGET)

# Target to force rebuild
FORCE:

usrlib:
	make -C sources/usr/lib	
	
rootfs: $(KERNEL_TARGET)
#	@test -d $(dir $(ROOTFS_PATH)/kernel/bin/) || mkdir -pm 775 $(dir $(ROOTFS_PATH)/kernel/bin/)
#	cp $(KERNEL_TARGET) $(ROOTFS_PATH)/kernel/bin/
	
ramdisk:
	make $(RAMDISK_TARGET)

application:
	make -C applications/$(APP)

$(OBJ_DIR)/%.o: %.S
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(ASM) $(AFLAGS) $(INCLUDES) -c -o $@ $<
		
$(OBJ_DIR)/%.o: %.s
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(ASM) $(AFLAGS) $(INCLUDES) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(KERNEL_TARGET): $(OBJS) $(OS_TARGET)
	$(LINK) $^ $(LDFLAGS) -o $@

$(BOOT_TARGET): $(BOOT_OBJS)
	$(LINK) -nostartfiles -nostdlib -T$(BOOT_LSCRIPT) -Map $(BOOT_MAP_FILE) $(BOOT_OBJS) -o $@

$(RAMDISK_TARGET): FORCE
	make rootfs
	make ramdiskmk
	rm -rf $(RAMDISK_TARGET)	
	tools/ramdiskmk/build/ramdiskmk $(RAMDISK_TARGET) $(ROOTFS_PATH)
	@echo Finished creating ramdisk file $(RAMDISK_TARGET)

tools: 
	@echo --------------------------------------------------------------------------------
	@echo Building necessary tools
	@echo --------------------------------------------------------------------------------
	@echo
	make ramdiskmk 
	make elfmerge
ifeq ($(TARGET), mini210s)		
	make mkv210_image
endif
	

ramdiskmk:
	make -C tools/$@
	
elfmerge:
	make -C tools/$@

mkv210_image:
ifeq ($(TARGET), mini210s)		
	make -C tools/$@
else
	echo "mkv210_image is only valid for mini210s target"
endif
	
write2sd: $(BOOT_TARGET) mkv210_image
	./tools/mkv210_image/write2sd.sh $(BOOT_TARGET)

clean:
	rm -rf $(DST)
	make -C applications/$(APP) clean
	make -C applications/G2D clean
	make -C applications/SystemMonitor clean
	make -C applications/test_os clean
	make -C applications/srt clean
	make -C applications/test_aperiodic clean
	make -C applications/test_rtc clean
	make -C sources/usr/lib clean
	make -C tools/elfmerge clean
	make -C tools/ramdiskmk clean
	make -C tools/mkv210_image clean
	rm -rf $(ROOTFS_PATH)/kernel/bin
	rm -rf $(ROOTFS_PATH)/applications/bin/

## Validate the arguments for build
ifneq ($(CONFIG),debug)
	ifneq ($(CONFIG),release)
		$(error CONFIG should be either debug or release)
	endif
endif

ifeq ($(TARGET),)
	$(error Missing TARGET specification)
endif
ifeq ($(SOC),)
	$(error Missing SOC specification)
endif
ifeq ($(CORE),)
	$(error Missing CORE specification)
endif
