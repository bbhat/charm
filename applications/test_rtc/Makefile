###################################################################################
##	
##						Copyright 2013 xxxxxxx, xxxxxxx
##	File:	Makefile
##	Author:	Bala B. (bhat.balasubramanya@gmail.com)
##	Description: Makefile for Applications
##
###################################################################################

CC:=arm-elf-gcc
LINK:=arm-elf-ld

## Initialize default arguments
TARGET		?=	tq2440
DST			?=	build
CONFIG		?=	debug
APP			?=	test_rtc

## Initialize dependent parameters
ifeq ($(TARGET), tq2440)
	SOC := s3c2440
endif

ifeq ($(SOC), s3c2440)
	CORE := arm920t
endif

ROOT_DIR		:=	$(realpath ../..)
BUILD_DIR		:=	$(DST)/$(CONFIG)
MAP_FILE		:=	$(BUILD_DIR)/$(APP).map
LINKERS_CRIPT	:=	$(ROOT_DIR)/scripts/$(TARGET)/applications/memmap_$(APP).ld
DEP_DIR			:=	$(BUILD_DIR)/dep
OBJ_DIR			:=	$(BUILD_DIR)/obj
BUILD_TARGET	:=	$(BUILD_DIR)/$(APP).elf
USR_LIB			:=	$(ROOT_DIR)/usr/lib/$(DST)/$(CONFIG)-$(TARGET)/usrlib.a

## Include source files
include $(wildcard *.mk)

## Include folders
INCLUDES		:=	$(ROOT_DIR)/usr/includes
INCLUDES		:=	$(addprefix -I , $(INCLUDES))

## Build a list of corresponding object files
OBJS			:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(SOURCES))))

## Build flags
AFLAGS		:=	-mcpu=arm920t -EL -g -mlittle-endian
CFLAGS		:=	-Wall -nostdinc -mcpu=$(CORE) -mlittle-endian
LDFLAGS		:=	-nostartfiles -nostdlib -T$(LINKERS_CRIPT) -Map $(MAP_FILE)
ifeq ($(CONFIG),debug)
	CFLAGS	:=	-g -O0 -D DEBUG $(CFLAGS)
else ifeq ($(CONFIG),release)
	CFLAGS	:=	-O2 -D RELEASE $(CFLAGS)
endif

## Rule specifications
.PHONY:	all clean

all:
	@echo --------------------------------------------------------------------------------
	@echo Starting $(APP) build with following parameters:
	@echo --------------------------------------------------------------------------------
	@echo TARGET=$(TARGET) 
	@echo SOC=$(SOC)
	@echo CONFIG=$(CONFIG)
	@echo APP=$(APP)
	@echo ROOT_DIR=$(ROOT_DIR)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OBJ_DIR=$(OBJ_DIR)
	@echo MAP_FILE=$(MAP_FILE)
	@echo SOURCES=$(SOURCES)
	@echo OBJS=$(OBJS)
	@echo INCLUDES=$(INCLUDES)
	@echo
	make $(BUILD_TARGET)

$(OBJ_DIR)/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(BUILD_TARGET): $(USR_LIB) $(OBJS)
	$(LINK) $^ $(LDFLAGS) -o $@

$(USR_LIB):
	make -C $(ROOT_DIR)/usr/lib
	
clean:
	rm -rf $(DST)

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
ifeq ($(APP),)
	$(error Missing APP specification)
endif
