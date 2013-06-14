BOOT_SOURCE_DIRS	:=	boot/mini210s

BOOT_SOURCES	+=	$(foreach srcdir, $(BOOT_SOURCE_DIRS), $(wildcard $(srcdir)/*.S))
