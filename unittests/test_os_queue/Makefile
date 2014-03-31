###################################################################################
##	
##						Copyright 2014 xxxxxxx, xxxxxxx
##	File:	Makefile
##	Author:	Bala B. (bhat.balasubramanya@gmail.com)
##	Description: Makefile for tests
##					These tests are written to run on Mac
##
###################################################################################

CC:=gcc

## Initialize default arguments
DST			?=	build
CONFIG		?=	debug
APP			?=	test_os_queue

OS_DIR			:=	$(realpath ../..)
BUILD_DIR		:=	$(DST)/$(CONFIG)
BUILD_TARGET	:=	$(BUILD_DIR)/$(APP)
SOURCES			:= 	$(wildcard *.c)

## Include folders
INCLUDES		:=	$(INCLUDES) $(OS_DIR)/sources/kernel
INCLUDES		:=	$(addprefix -I , $(INCLUDES))

## Build flags
CFLAGS		:= -Wall -arch i386
ifeq ($(CONFIG),debug)
	CFLAGS	:=	-ggdb -O0 -D DEBUG $(CFLAGS)
else ifeq ($(CONFIG),release)
	CFLAGS	:=	-O2 -D RELEASE $(CFLAGS)
endif

## Rule specifications
.PHONY:	all clean

all: $(BOOT_OBJS)
	@echo --------------------------------------------------------------------------------
	@echo Starting build with following parameters:
	@echo --------------------------------------------------------------------------------
	@echo CONFIG=$(CONFIG)
	@echo APP=$(APP)
	@echo ROOT_DIR=$(ROOT_DIR)
	@echo BUILD_DIR=$(BUILD_DIR)
	@echo SOURCES=$(SOURCES)
	@echo INCLUDES=$(INCLUDES)
	@echo
	make $(BUILD_TARGET)

$(BUILD_TARGET): $(SOURCES)
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -rf $(BUILD_DIR)

## Validate the arguments for build
ifneq ($(CONFIG),debug)
	ifneq ($(CONFIG),release)
		$(error CONFIG should be either debug or release)
	endif
endif

ifeq ($(APP),)
	$(error Missing APP specification)
endif
