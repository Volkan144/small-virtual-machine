#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

/* Hardcoded program for now: counts down from 10 to 1.
 * Starting with v3, programs are loaded from a file produced by the assembler. */
static const int32_t program[] = {
    /*  0 */ OP_PUSH, 5,
    /*  2 */ OP_CALL, 6,       // fact(5)
    /*  4 */ OP_PRINT,         // -> 120
    /*  5 */ OP_HALT,

    /* fact: expects n on top of the stack, leaves n! */
    /*  6 */ OP_DUP,           // [n, n]
    /*  7 */ OP_JZ, 17,        // n == 0 -> base case
    /*  9 */ OP_DUP,           // [n, n]
    /* 10 */ OP_PUSH, 1,
    /* 12 */ OP_SUB,           // [n, n-1]
    /* 13 */ OP_CALL, 6,       // [n, (n-1)!]
    /* 15 */ OP_MUL,           // [n!]
    /* 16 */ OP_RET,

    /* base case */
    /* 17 */ OP_POP,           // drop the 0
    /* 18 */ OP_PUSH, 1,       // 0! = 1
    /* 20 */ OP_RET,
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
