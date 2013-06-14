BOOT_SOURCE_DIRS	:=	boot/tq2440

BOOT_SOURCES	+=	$(foreach srcdir, $(BOOT_SOURCE_DIRS), $(wildcard $(srcdir)/*.S))
