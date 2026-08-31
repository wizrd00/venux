.PHONY: libc

LIBC_CFLAGS := -I/libc/include

LIBC_SRC_FILES := $(wildcard libc/src/*.c)
LIBC_OBJ_FILES := $(patsubst libc/src/%.c, $(BIN_DIR)/%.c.o, $(LIBC_SRC_FILES))

libc : $(BIN_DIR)/libc.a

$(BIN_DIR)/libc.a : $(LIBC_OBJ_FILES)
	$(AR) rcs $@ $(LIBC_OBJ_FILES)

$(BIN_DIR)/%.c.o : libc/src/%.c libc/include/%.h
	$(CC) $(CFLAGS) $(LIBC_CFLAGS) -c -o $@ $<
