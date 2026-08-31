.PHONY: kernel clean-kernel

KERN_CFLAGS := -Ikernel/include

SRC_FILES := $(wildcard kernel/src/*.c)
ASM_FILES := $(wildcard kernel/$(ARCH)/*.s)
OBJ_FILES := $(patsubst kernel/src/%.c, $(BIN_DIR)/%.c.o, $(SRC_FILES)) $(patsubst kernel/$(ARCH)/%.s, $(BIN_DIR)/%.s.o, $(ASM_FILES))

kernel : $(BIN_DIR)/venux.a

$(BIN_DIR)/venux.a : $(OBJ_FILES)
	$(AR) rcs $@ $(OBJ_FILES)

$(BIN_DIR)/%.c.o : kernel/src/%.c kernel/include/%.h
	$(CC) $(CFLAGS) $(KERN_CFLAGS) -c -o $@ $<

$(BIN_DIR)/%.s.o : kernel/$(ARCH)/%.s
	$(AS) $(SFLAGS) -o $@ $<

clean-kernel :
	rm venux.elf
