CC:=gcc

BIN:=build/mkv210_image
OBJ:=build/mkv210_image.o
SRC:=mkv210_image.c

ROOT_DIR	:=	$(realpath ../..)
CFLAGS		:=	-Wall

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $(BIN) $(OBJ)

build/%.o: %.c
	@test -d $(dir $@) || mkdir -pm 775 $(dir $@)
	$(CC) -g -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(BIN) $(OBJ)