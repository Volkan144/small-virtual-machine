# Small Virtual Machine

A small stack-based virtual machine and a two-pass assembler, written in C11 without external dependencies.

Programs are written in a simple assembly language, translated into bytecode by the assembler, and executed by the VM:

```
fact.asm  ──▶  build/asm  ──▶  fact.bin  ──▶  build/vm  ──▶  120
```

## Build & Run

Requires `gcc` and `make`.

```sh
make            # build the VM            (build/vm)
make asm        # build the assembler     (build/asm)
make test       # build and run all tests
make clean      # remove build artifacts
```

Assemble and run a program:

```sh
./build/asm programs/fact.asm fact.bin
./build/vm fact.bin
```

## Example

Recursive factorial (`programs/fact.asm`):

```asm
        push 5
        call fact 1      ; call fact with 1 argument
        print            ; -> 120
        halt

fact:   dup
        jz base          ; n == 0 -> base case
        dup
        push 1
        sub              ; n - 1
        call fact 1      ; fact(n - 1)
        mul              ; n * fact(n - 1)
        ret

base:   push 1           ; 0! = 1
        ret
```

## Instruction Set

Stack effects are written as `before -> after`, with the top of the stack on the right.

| Instruction       | Opcode | Stack effect      | Description |
|-------------------|:------:|-------------------|-------------|
| `halt`            | 0      |                   | Stop execution |
| `push n`          | 1      | `-> n`            | Push the constant `n` |
| `pop`             | 2      | `a ->`            | Discard the top value |
| `add`             | 3      | `a b -> a+b`      | Addition |
| `sub`             | 4      | `a b -> a-b`      | Subtraction |
| `mul`             | 5      | `a b -> a*b`      | Multiplication |
| `dup`             | 6      | `a -> a a`        | Duplicate the top value |
| `jmp label`       | 7      |                   | Unconditional jump |
| `jz label`        | 8      | `a ->`            | Pop `a`, jump if `a == 0` |
| `print`           | 9      | `a -> a`          | Print the top value without popping it |
| `call label n`    | 10     |                   | Call a function with `n` arguments |
| `ret`             | 11     | `... r -> r`      | Return `r` and discard the callee's frame |
| `load i`          | 12     | `-> v`            | Push local variable `i` of the current frame |
| `store i`         | 13     | `v ->`            | Pop `v` into local variable `i` |

## Calling Convention

- The caller pushes the arguments, then executes `call label n`.
- Inside the function, the arguments are local variables `0` to `n-1` (`load 0`, `load 1`, ...). Further locals can be created by pushing values at the start of the function.
- A function leaves exactly one result on top of the stack and executes `ret`.
- `ret` removes the arguments and everything else in the frame, and pushes the result. For the caller, `call` behaves like a single instruction that replaces `n` arguments with one result.

Example with two arguments:

```asm
        push 3
        push 4
        call sumsq 2     ; -> 25
        print
        halt

sumsq:  load 0           ; a * a + b * b
        load 0
        mul
        load 1
        load 1
        mul
        add
        ret
```

## Assembly Syntax

- One instruction per line, arguments separated by whitespace
- `name:` defines a label at the address of the next instruction; it may stand on its own line or before an instruction
- Labels may be used before they are defined (forward references)
- `;` starts a comment that runs to the end of the line
- Numbers are decimal and may be negative
- Unix and Windows line endings are both accepted

## Design

**Separate call stack.** Return addresses are not stored on the data stack. If they were, a function that leaves its result on top of the stack would make `ret` pop the result instead of the return address. A second stack keeps control flow and data apart, so a buggy program can corrupt its data but not the return path.

**Frame pointer.** On `call`, the VM records where the arguments of the new frame begin. `load`/`store` index relative to that position, so local variable `0` always refers to the same slot, no matter how many temporary values the function has pushed since. The caller's frame pointer is saved on the call stack and restored by `ret`, which is what makes nested and recursive calls work.

**Two-pass assembler.** A label can be used before it is defined (`call fact` appears before `fact:`). The first pass only records the address of every label, the second pass emits the final bytecode with all labels resolved.

**Defensive execution.** Every stack access, operand fetch and jump target is bounds-checked. Malformed bytecode stops the VM with an error such as `stack underflow` or `program counter out of bounds` instead of reading or writing outside its memory.

**Testable core.** The VM (`vm_run`) and the assembler (`assemble`) are plain functions without file or console handling. The command-line programs are thin wrappers, and the unit tests call the core functions directly.

## Bytecode Format

A `.bin` file is a flat sequence of 32-bit little-endian integers with no header: each opcode is followed by its operands. `push 5` is stored as the bytes `01 00 00 00 05 00 00 00`.

## Limits

| Resource         | Limit |
|------------------|-------|
| Data stack       | 256 values |
| Call stack       | 256 nested calls |
| Program size     | 65 536 words |
| Labels           | 256, up to 31 characters |

## Project Structure

```
src/          VM core (vm.c, vm.h, opcodes.h) and the VM command-line program (main.c)
assembler/    assembler core (assembler.c, assembler.h) and its command-line program
tests/        unit tests for the VM and the assembler
programs/     example programs
```

## Roadmap

- [x] **v1 – Stack VM:** arithmetic, stack operations, jumps
- [x] **v2 – Function calls:** `call`/`ret` with a separate call stack
- [x] **v2.5 – Local variables:** stack frames, frame pointer, `load`/`store`
- [x] **v3 – Assembler:** two-pass assembler with labels and comments
- [ ] **v4 – Performance:** compare `switch` dispatch with computed goto