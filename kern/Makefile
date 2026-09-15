.PHONY: kernel clean-kernel

KERN_CFLAGS := -Ikern/include \
	-fPIE

KERN_SRC_FILES := $(wildcard kern/src/*.c)
KERN_ASM_FILES := $(wildcard kern/$(ARCH)/*.s)
KERN_OBJ_FILES := $(patsubst kern/src/%.c, $(BIN_DIR)/%.c.o, $(KERN_SRC_FILES)) $(patsubst kern/$(ARCH)/%.s, $(BIN_DIR)/%.s.o, $(KERN_ASM_FILES))

kernel : $(BIN_DIR)/venux.a

$(BIN_DIR)/venux.a : $(KERN_OBJ_FILES)
	$(AR) rcs $@ $(KERN_OBJ_FILES)

$(BIN_DIR)/%.c.o : kern/src/%.c $(wildcard kern/include/*)
	$(CC) $(CFLAGS) $(KERN_CFLAGS) -c -o $@ $<

$(BIN_DIR)/%.s.o : kern/$(ARCH)/%.s
	$(AS) $(SFLAGS) -o $@ $<

clean-kernel :
	rm venux.elf
