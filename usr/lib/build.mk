SOURCE_DIRS	:=	

SOURCES		+=	$(wildcard *.c) $(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.c))
SOURCES		+=	$(wildcard *.S) $(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.S))