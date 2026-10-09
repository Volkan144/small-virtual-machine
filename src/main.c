#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

/* Hardcoded program for now: counts down from 10 to 1.
 * Starting with v3, programs are loaded from a file produced by the assembler. */
static const int32_t program[] = {
    /*  0 */ OP_PUSH, 5,
    /*  2 */ OP_CALL, 7, 1,     // fact(5)
    /*  5 */ OP_PRINT,          // -> 120
    /*  6 */ OP_HALT,

    /* fact: expects n as its only argument, returns n! */
    /*  7 */ OP_DUP,
    /*  8 */ OP_JZ, 19,
    /* 10 */ OP_DUP,
    /* 11 */ OP_PUSH, 1,
    /* 13 */ OP_SUB,
    /* 14 */ OP_CALL, 7, 1,
    /* 17 */ OP_MUL,
    /* 18 */ OP_RET,

    /* base case: RET cleans up the 0 */
    /* 19 */ OP_PUSH, 1,
    /* 21 */ OP_RET,
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
