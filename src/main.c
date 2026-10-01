#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

/* Hardcoded program for now: counts down from 10 to 1.
 * Starting with v3, programs are loaded from a file produced by the assembler. */
static const int32_t program[] = {
    /*  0 */ OP_PUSH, 10,
    /*  2 */ OP_PRINT,
    /*  3 */ OP_PUSH, 1,
    /*  5 */ OP_SUB,
    /*  6 */ OP_DUP,
    /*  7 */ OP_JZ, 11,
    /*  9 */ OP_JMP, 2,
    /* 11 */ OP_HALT,
};

int main(void) {
    VM vm;
    vm_init(&vm, program, sizeof program / sizeof program[0]);

    VMResult r = vm_run(&vm);
    if (r != VM_OK) {
        fprintf(stderr, "VM error at pc=%zu: %s\n", vm.pc, vm_result_str(r));
        return 1;
    }
    return 0;
}
