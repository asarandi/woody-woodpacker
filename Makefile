CFLAGS += -Wall -Werror -Wextra -I include/
LDFLAGS += -O1

BIN = woody_woodpacker
SRC = src/decrypt.c src/encrypt.c src/load.c src/main.c src/segment.c
SRC += src/g_decryptor.c # created dynamically
OBJ = $(SRC:.c=.o)

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $^

src/g_decryptor.c: src/g_decryptor.bin
	xxd -i -c 8 -n g_decryptor $< $@

src/g_decryptor.bin: src/decryptor.s
	nasm -f bin $< -o $@

clean:
	rm -f $(OBJ) src/g_decryptor.c src/g_decryptor.bin

fclean: clean
	rm -f $(BIN)
