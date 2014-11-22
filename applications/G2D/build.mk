SOURCE_DIRS	:= 

SOURCES		+=	 $(wildcard *.c) $(foreach srcdir, $(SOURCE_DIRS), $(wildcard $(srcdir)/*.c))
