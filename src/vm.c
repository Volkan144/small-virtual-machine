#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

void vm_init(VM *vm, const int32_t *code, size_t code_len) {
    vm->code = code;
    vm->code_len = code_len;
    vm->pc = 0;
    vm->sp = 0;
}

/* TODO v1: implement push/pop with overflow/underflow checks.
 * Hint: return a VMResult, pass the popped value out via a pointer. */
static VMResult push(VM *vm, int32_t value) {
    (void)vm; (void)value;
    return VM_OK;
}

static VMResult pop(VM *vm, int32_t *out) {
    (void)vm; (void)out;
    return VM_OK;
}

VMResult vm_run(VM *vm) {
    (void)push; (void)pop; /* remove once they are used */

    for (;;) {
        if (vm->pc >= vm->code_len)
            return VM_ERR_PC_OUT_OF_BOUNDS;

        /* fetch */
        int32_t op = vm->code[vm->pc++];

        /* decode + execute */
        switch (op) {
        case OP_HALT:
            return VM_OK;

        /* TODO v1: implement the remaining opcodes from opcodes.h */

        default:
            return VM_ERR_UNKNOWN_OPCODE;
        }
    }
}

const char *vm_result_str(VMResult r) {
    switch (r) {
    case VM_OK:                   return "OK";
    case VM_ERR_STACK_OVERFLOW:   return "stack overflow";
    case VM_ERR_STACK_UNDERFLOW:  return "stack underflow";
    case VM_ERR_UNKNOWN_OPCODE:   return "unknown opcode";
    case VM_ERR_PC_OUT_OF_BOUNDS: return "program counter out of bounds";
    }
    return "?";
}
