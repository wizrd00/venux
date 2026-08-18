.PHONY: libc

LIBC_CFLAGS := -I/libc/include

SRC_FILES := $(wildcard libc/src/*.c)
OBJ_FILES := $(patsubst libc/src/%.c, $(BIN_DIR)/%.o, $(SRC_FILES))

libc : $(BIN_DIR)/libc.a

$(BIN_DIR)/libc.a : $(OBJ_FILES)
	$(AR) rcs $@ $(OBJ_FILES)

$(BIN_DIR)/%.o : libc/src/%.c libc/include/%.h
	$(CC) $(CFLAGS) $(LIBC_CFLAGS) -c -o $@ $<
