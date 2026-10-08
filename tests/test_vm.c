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

static void test_sub_order(void) {
    const int32_t code[] = { OP_PUSH, 3, OP_PUSH, 4, OP_SUB, OP_HALT };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "SUB runs without error");
    CHECK(vm.sp == 1, "SUB leaves exactly one value");
    CHECK(vm.stack[0] == -1, "SUB computes 3 - 4 = -1");
}
static void test_underflow(void) {
    const int32_t code[] = { OP_ADD, OP_HALT };
    VM vm; vm_init(&vm, code, sizeof code / sizeof code[0]);
    CHECK(vm_run(&vm) == VM_ERR_STACK_UNDERFLOW, "ADD on empty stack reports underflow");
}
static void test_overflow(void) {
    int32_t code[(STACK_SIZE + 1) * 2 + 1];
    size_t i = 0;

    for (int k = 0; k < STACK_SIZE + 1; k++) {
        code[i++] = OP_PUSH;
        code[i++] = k;
}

    code[i++] = OP_HALT;

    VM vm;
    vm_init(&vm, code, i);
    CHECK(vm_run(&vm) == VM_ERR_STACK_OVERFLOW, "Pushing more elements than STACK_SIZE into stack reports overflow");
    CHECK(vm.sp == STACK_SIZE, "overflow leaves the stack exactly full");
}

static void test_jmp(void) {
    const int32_t code[] = {
        /* 0 */ OP_JMP, 4,
        /* 2 */ OP_PUSH, 99,
        /* 4 */ OP_HALT,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "JMP runs without error");
    CHECK(vm.sp == 0, "JMP skips the instruction in between");
}

static void test_jz_taken(void) {
    const int32_t code[] = {
        /* 0 */ OP_PUSH, 0,
        /* 2 */ OP_JZ, 6,
        /* 4 */ OP_PUSH, 99,
        /* 6 */ OP_HALT,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "JZ with 0 runs without error");
    CHECK(vm.sp == 0, "JZ with 0 jumps and pops the condition");
}

static void test_jz_not_taken(void) {
    const int32_t code[] = {
        /* 0 */ OP_PUSH, 1,
        /* 2 */ OP_JZ, 6,
        /* 4 */ OP_PUSH, 99,
        /* 6 */ OP_HALT,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "JZ with non-zero runs without error");
    CHECK(vm.sp == 1, "JZ with non-zero pops the condition");
    CHECK(vm.stack[0] == 99, "JZ with non-zero does not jump");
}

static void test_jmp_out_of_bounds(void) {
    const int32_t code[] = { OP_JMP, 100, OP_HALT };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_ERR_PC_OUT_OF_BOUNDS, "JMP to invalid target is rejected");
}

int main(void) {
    test_halt();
    test_unknown_opcode();
    test_sub_order();
    test_underflow();
    test_overflow();
    test_jmp();
    test_jz_taken();
    test_jz_not_taken();
    test_jmp_out_of_bounds();
    printf("\n%s (%d failures)\n", failures ? "FAILED" : "ALL PASSED", failures);
    return failures ? 1 : 0;
}