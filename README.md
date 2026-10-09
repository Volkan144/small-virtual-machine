# Small Virtual Machine

A small stack-based virtual machine written in C.

## Build & Run

```sh
make        # builds build/vm
make run    # runs the example program
make test   # runs the tests
make clean
```

## Roadmap

- [x] **v1 – Stack VM:** PUSH, POP, ADD, SUB, MUL, DUP, JMP, JZ, PRINT, HALT
- [x] **v2 – Function calls:** CALL/RET, stack frames, local variables
- [ ] **v3 – Assembler:** translate `.asm` source into bytecode, with label support
- [ ] **v4 – LC-3:** emulate a real instruction set architecture
- [ ] **v5 – Performance:** compare dispatch techniques (switch vs. computed goto)

## Structure

```
src/        VM core (vm.c/.h, opcodes.h, main.c)
tests/      unit tests
assembler/  assembler (v3)
programs/   example programs in VM assembly
```
