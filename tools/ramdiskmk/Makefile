CC:=gcc

BIN:=build/ramdiskmk
OBJ:=build/ramdiskmk.o
SRC:=ramdiskmk.c

ROOT_DIR	:= 	$(realpath ../..)

INCLUDES 	:= 	$(ROOT_DIR)/sources/filesystem/ramdisk 	$(ROOT_DIR)/sources/kernel

INCLUDES	:=	$(addprefix -I ,$(INCLUDES))
CFLAGS		:=	-Wall

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

build/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -g -D__APPLE__ -c $(INCLUDES) $(CFLAGS) -o $@ $<

clean:
	rm -rf build/