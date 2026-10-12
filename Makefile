CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g -O0
BUILD   := build

VM_SRC  := src/vm.c
VM_OBJ  := $(BUILD)/vm.o

.PHONY: all run test asm clean

all: $(BUILD)/vm

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/vm.o: src/vm.c src/vm.h src/opcodes.h | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/vm: src/main.c $(VM_OBJ) | $(BUILD)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD)/test_vm: tests/test_vm.c $(VM_OBJ) | $(BUILD)
	$(CC) $(CFLAGS) -Isrc $^ -o $@

$(BUILD)/test_asm: tests/test_asm.c $(BUILD)/assembler.o | $(BUILD)
	$(CC) $(CFLAGS) -Iassembler -Isrc $^ -o $@

$(BUILD)/assembler.o: assembler/assembler.c assembler/assembler.h src/opcodes.h | $(BUILD)
	$(CC) $(CFLAGS) -Isrc -c  $< -o $@

$(BUILD)/asm: assembler/asm.c $(BUILD)/assembler.o | $(BUILD)
	$(CC) $(CFLAGS)  $^ -o $@

run: $(BUILD)/vm
	./$(BUILD)/vm $(PROG)

test: $(BUILD)/test_vm $(BUILD)/test_asm
	./$(BUILD)/test_vm
	./$(BUILD)/test_asm
asm: $(BUILD)/asm

clean:
	rm -rf $(BUILD)
