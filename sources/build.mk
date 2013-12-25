
## Build a list of Source directories

SOURCE_DIRS	+=	sources/kernel

SOURCE_DIRS	+=	sources/arm/$(CORE)
SOURCE_DIRS	+=	sources/arm/common

SOURCE_DIRS	+=	sources/soc/common/drivers/rtc
SOURCE_DIRS	+=	sources/soc/common/drivers/timer

SOURCE_DIRS	+=	sources/soc/$(SOC)/drivers/uart
SOURCE_DIRS	+=	sources/soc/$(SOC)
ifeq ($(SOC), s5pv210)
	SOURCE_DIRS	+=	sources/soc/$(SOC)/drivers/vic
endif

SOURCE_DIRS	+=	sources/target/$(TARGET)

SOURCE_DIRS	+=	sources/mmu/common
SOURCE_DIRS	+=	sources/mmu/$(CORE)
SOURCE_DIRS	+=	sources/mmu/$(CORE)

SOURCE_DIRS	+=	sources/drivers
SOURCE_DIRS	+=	sources/filesystem
SOURCE_DIRS	+=	sources/filesystem/ramdisk
SOURCE_DIRS	+=	sources/utilities
SOURCE_DIRS	+=	sources/loader
SOURCE_DIRS	+=	sources/memmgr

## Build a list of Include directories

INCLUDES	+=	sources/kernel

INCLUDES	+=	sources/arm/$(CORE)
INCLUDES	+=	sources/arm/common

INCLUDES	+=	sources/soc/common/drivers/rtc
INCLUDES	+=	sources/soc/common/drivers/timer
INCLUDES	+=	sources/soc/common/drivers/uart
INCLUDES	+=	sources/soc/$(SOC)
ifeq ($(SOC), s5pv210)
	INCLUDES	+=	sources/soc/$(SOC)/drivers/vic
endif

INCLUDES	+=	sources/target/$(TARGET)

INCLUDES	+=	sources/mmu/common
INCLUDES	+=	sources/mmu/$(CORE)
INCLUDES	+=	sources/mmu/$(CORE)

INCLUDES	+=	sources/drivers
INCLUDES	+=	sources/filesystem
INCLUDES	+=	sources/filesystem/ramdisk
INCLUDES	+=	sources/utilities
INCLUDES	+=	sources/loader
INCLUDES	+=	sources/memmgr

## Build a list of Source files

SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.c))
SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.s))
SOURCES		+=	$(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.S))

