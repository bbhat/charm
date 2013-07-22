###################################################################################
##	
##						Copyright 2013 xxxxxxx, xxxxxxx
##	File:	Makefile
##	Author:	Bala B. (bhat.balasubramanya@gmail.com)
##	Description: Makefile for Applications
##
###################################################################################

CC:=arm-elf-gcc
AR:=arm-elf-ar

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

ROOT_DIR		:=	$(realpath ../..)
BUILD_DIR		:=	$(DST)/$(CONFIG)-$(TARGET)
DEP_DIR			:=	$(BUILD_DIR)/dep
OBJ_DIR			:=	$(BUILD_DIR)/obj
BUILD_TARGET	:=	$(BUILD_DIR)/usrlib.a

## Include source files
include $(wildcard *.mk)

## Include folders
INCLUDES		:=	$(ROOT_DIR)/usr/includes
INCLUDES		:=	$(addprefix -I , $(INCLUDES))

## Build a list of corresponding object files
OBJS			:=	$(addsuffix .o, $(basename $(addprefix $(OBJ_DIR)/, $(SOURCES))))

## Build flags
AFLAGS		:=	-mcpu=arm920t -EL -g
CFLAGS		:=	-Wall -nostdinc -mcpu=$(CORE) -mlittle-endian
ifeq ($(CONFIG),debug)
	CFLAGS	:=	-g -O0 -D DEBUG $(CFLAGS)
else ifeq ($(CONFIG),release)
	CFLAGS	:=	-O2 -D RELEASE $(CFLAGS)
endif

## Rule specifications
.PHONY:	all clean

all:
	@echo --------------------------------------------------------------------------------
	@echo Building user libraries with following parameters
	@echo --------------------------------------------------------------------------------
	@echo TARGET=$(TARGET) 
	@echo SOC=$(SOC)
	@echo CONFIG=$(CONFIG)
	@echo ROOT_DIR=$(ROOT_DIR)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo OBJ_DIR=$(OBJ_DIR)
	@echo SOURCES=$(SOURCES)
	@echo OBJS=$(OBJS)
	@echo INCLUDES=$(INCLUDES)
	@echo
	make $(BUILD_TARGET)

$(OBJ_DIR)/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $@

$(BUILD_TARGET): $(OBJS)
	@echo "Building :" $@
	$(AR) -r $@ $^

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
