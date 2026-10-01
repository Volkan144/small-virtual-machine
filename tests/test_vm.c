#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } \
    else         { printf("ok:   %s\n", msg); } \
} while (0)

static void test_halt(void) {
    const int32_t code[] = { OP_HALT };
    VM vm; vm_init(&vm, code, 1);
    CHECK(vm_run(&vm) == VM_OK, "HALT stops cleanly");
}

static void test_unknown_opcode(void) {
    const int32_t code[] = { 999 };
    VM vm; vm_init(&vm, code, 1);
    CHECK(vm_run(&vm) == VM_ERR_UNKNOWN_OPCODE, "unknown opcode is detected");
}

/* TODO v1: tests for PUSH/ADD/SUB/MUL, stack underflow, JZ, ... */

int main(void) {
    test_halt();
    test_unknown_opcode();
    printf("\n%s (%d failures)\n", failures ? "FAILED" : "ALL PASSED", failures);
    return failures ? 1 : 0;
}
