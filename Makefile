###################################################################################
##	
##						Copyright 2009-2013 xxxxxxx, xxxxxxx
##	File:	Makefile
##	Author:	Bala B. (bhat.balasubramanya@gmail.com)
##	Description: Makefile for OS files
##
###################################################################################

ASM=arm-elf-gcc
CC:=arm-elf-gcc
LINK:=arm-elf-ld
LIBPATH:=/usr/local/dev-arm/i386-Darwin-arm-gcc-4.6.1/lib/gcc/arm-elf/4.6.1

## Initialize default arguments
TARGET		?=	mini210s
DST			?=	build
CONFIG		?=	debug

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

## Create INCLUDES 
include $(wildcard includes/*.mk)
INCLUDES		:=	$(addprefix -I ,includes $(INCLUDES))

## Build list of source and object files
SUBDIRS			:=	sources main
SOURCES			:=	$(wildcard *.c)

## Include Boot source files
include $(wildcard boot/$(TARGET)/*.mk)

## Include sources from other directories
include $(foreach sdir, $(SUBDIRS), $(wildcard $(sdir)/*.mk))

## Build a list of corresponding object files
OBJS			:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(SOURCES))))
BOOT_OBJS		:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(BOOT_SOURCES))))

## Build flags
AFLAGS		:=	-mcpu=$(CORE) -g
CFLAGS		:=	-Wall -nostdinc -mcpu=$(CORE) -mlittle-endian
LDFLAGS		:=	-nostartfiles -nostdlib -T$(LINKERS_SCRIPT) -Map $(MAP_FILE) -L $(LIBPATH) -lgcc
ifeq ($(CONFIG),debug)
	CFLAGS	:=	-g -O0 -D DEBUG $(CFLAGS)
else ifeq ($(CONFIG),release)
	CFLAGS	:=	-O2 -D RELEASE $(CFLAGS)
endif


## Rule specifications
.PHONY:	all boot dep clean ramdiskmk elfmerge tools kernel usrlib ramdisk

all:
	make boot
	make kernel
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

usrlib:
	make -C usr/lib	
	
rootfs: $(KERNEL_TARGET)
	@test -d $(dir $(ROOTFS_PATH)/kernel/bin/) || mkdir -pm 775 $(dir $(ROOTFS_PATH)/kernel/bin/)
	cp $(KERNEL_TARGET) $(ROOTFS_PATH)/kernel/bin/
	
ramdisk: 
	make $(RAMDISK_TARGET)

$(OBJ_DIR)/%.o: %.S
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(ASM) $(AFLAGS) -c -o $@ $<
		
$(OBJ_DIR)/%.o: %.s
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(ASM) $(AFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(KERNEL_TARGET): $(OBJS) $(OS_TARGET)
	$(LINK) $^ $(LDFLAGS) -o $@

$(BOOT_TARGET): $(BOOT_OBJS)
	$(LINK) -nostartfiles -nostdlib -T$(BOOT_LSCRIPT) -Map $(BOOT_MAP_FILE) $(BOOT_OBJS) -o $@

$(RAMDISK_TARGET):
	make rootfs
	make ramdiskmk
	tools/ramdiskmk/build/ramdiskmk $(BUILD_DIR)/ramdisk.img $(ROOTFS_PATH)
	@echo Finished creating ramdisk file $(BUILD_DIR)/ramdisk.img

tools: 
	@echo --------------------------------------------------------------------------------
	@echo Building necessary tools
	@echo --------------------------------------------------------------------------------
	@echo
	make ramdiskmk 
	make elfmerge

ramdiskmk:
	make -C tools/$@
	
elfmerge:
	make -C tools/$@

clean:
	rm -rf $(DST)
	make -C usr/lib clean
	make -C tools/elfmerge clean
	make -C tools/ramdiskmk clean
	rm -rf $(ROOTFS_PATH)/kernel/bin

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
