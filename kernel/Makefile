.PHONY: kernel clean-kernel

KERN_CFLAGS := -Ikernel/include \
	-fPIE

KERN_SRC_FILES := $(wildcard kernel/src/*.c)
KERN_ASM_FILES := $(wildcard kernel/$(ARCH)/*.s)
KERN_OBJ_FILES := $(patsubst kernel/src/%.c, $(BIN_DIR)/%.c.o, $(KERN_SRC_FILES)) $(patsubst kernel/$(ARCH)/%.s, $(BIN_DIR)/%.s.o, $(KERN_ASM_FILES))

kernel : $(BIN_DIR)/venux.a

$(BIN_DIR)/venux.a : $(KERN_OBJ_FILES)
	$(AR) rcs $@ $(KERN_OBJ_FILES)

$(BIN_DIR)/%.c.o : kernel/src/%.c kernel/include/%.h
	$(CC) $(CFLAGS) $(KERN_CFLAGS) -c -o $@ $<

$(BIN_DIR)/%.s.o : kernel/$(ARCH)/%.s
	$(AS) $(SFLAGS) -o $@ $<

clean-kernel :
	rm venux.elf
