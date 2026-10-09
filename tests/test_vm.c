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

static void test_call_ret_factorial(void) {
    const int32_t code[] = {
        /*  0 */ OP_PUSH, 5,
        /*  2 */ OP_CALL, 6, 1,
        /*  5 */ OP_HALT,

        /* fact: expects n as its only argument, returns n! */
        /*  6 */ OP_DUP,
        /*  7 */ OP_JZ, 18,
        /*  9 */ OP_DUP,
        /* 10 */ OP_PUSH, 1,
        /* 12 */ OP_SUB,
        /* 13 */ OP_CALL, 6, 1,
        /* 16 */ OP_MUL,
        /* 17 */ OP_RET,

        /* base case: RET cleans up the 0 */
        /* 18 */ OP_PUSH, 1,
        /* 20 */ OP_RET,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "recursive factorial runs without error");
    CHECK(vm.sp == 1, "factorial leaves exactly one value");
    CHECK(vm.stack[0] == 120, "factorial computes 5! = 120");
    CHECK(vm.csp == 0, "all calls returned");
}

static void test_call_stack_overflow(void) {
    const int32_t code[] = { OP_CALL, 0, 0 };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_ERR_CALL_STACK_OVERFLOW, "infinite recursion reports call stack overflow");
    CHECK(vm.csp == CALL_STACK_SIZE, "call stack overflow leaves the call stack exactly full");
}

static void test_ret_without_call(void) {
    const int32_t code[] = { OP_RET, OP_HALT };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_ERR_CALL_STACK_UNDERFLOW, "RET without CALL reports call stack underflow");
}

static void test_load_args(void) {
    const int32_t code[] = {
        /*  0 */ OP_PUSH, 3,
        /*  2 */ OP_PUSH, 4,
        /*  4 */ OP_CALL, 8, 2,
        /*  7 */ OP_HALT,

        /* sum_of_squares(a, b) = a*a + b*b */
        /*  8 */ OP_LOAD, 0,
        /* 10 */ OP_LOAD, 0,
        /* 12 */ OP_MUL,
        /* 13 */ OP_LOAD, 1,
        /* 15 */ OP_LOAD, 1,
        /* 17 */ OP_MUL,
        /* 18 */ OP_ADD,
        /* 19 */ OP_RET,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "LOAD reads arguments without error");
    CHECK(vm.sp == 1, "RET removes the arguments and leaves only the result");
    CHECK(vm.stack[0] == 25, "sum_of_squares(3, 4) = 25");
}

static void test_store(void) {
    const int32_t code[] = {
        /*  0 */ OP_PUSH, 10,
        /*  2 */ OP_CALL, 6, 1,
        /*  5 */ OP_HALT,

        /* f(n): n = n + 1; return n */
        /*  6 */ OP_LOAD, 0,
        /*  8 */ OP_PUSH, 1,
        /* 10 */ OP_ADD,
        /* 11 */ OP_STORE, 0,
        /* 13 */ OP_LOAD, 0,
        /* 15 */ OP_RET,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "STORE runs without error");
    CHECK(vm.stack[0] == 11, "STORE overwrites a local variable");
}

static void test_ret_restores_frame(void) {
    const int32_t code[] = {
        /*  0 */ OP_PUSH, 2,
        /*  2 */ OP_CALL, 6, 1,
        /*  5 */ OP_HALT,

        /* outer(x) = x + inner(10) + x */
        /*  6 */ OP_LOAD, 0,
        /*  8 */ OP_PUSH, 10,
        /* 10 */ OP_CALL, 18, 1,
        /* 13 */ OP_LOAD, 0,       // must still be x, not inner's argument
        /* 15 */ OP_ADD,
        /* 16 */ OP_ADD,
        /* 17 */ OP_RET,

        /* inner(n) = n + 1 */
        /* 18 */ OP_LOAD, 0,
        /* 20 */ OP_PUSH, 1,
        /* 22 */ OP_ADD,
        /* 23 */ OP_RET,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_OK, "nested calls with locals run without error");
    CHECK(vm.stack[0] == 15, "RET restores the caller's frame pointer");
    CHECK(vm.fp == 0, "frame pointer is back at 0 after all returns");
}

static void test_load_invalid(void) {
    const int32_t code[] = {
        /* 0 */ OP_PUSH, 1,
        /* 2 */ OP_CALL, 6, 1,
        /* 5 */ OP_HALT,
        /* 6 */ OP_LOAD, 1,        // only slot 0 exists
        /* 8 */ OP_RET,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_ERR_INVALID_LOCAL, "LOAD outside the frame is rejected");
}

static void test_store_into_popped_slot(void) {
    const int32_t code[] = {
        /*  0 */ OP_PUSH, 1,
        /*  2 */ OP_CALL, 6, 1,
        /*  5 */ OP_HALT,
        /*  6 */ OP_PUSH, 99,
        /*  8 */ OP_STORE, 1,      // slot 1 is the value being stored
        /* 10 */ OP_RET,
    };
    VM vm;
    vm_init(&vm, code, sizeof code / sizeof code[0]);

    CHECK(vm_run(&vm) == VM_ERR_INVALID_LOCAL, "STORE into the slot it pops from is rejected");
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
    test_call_ret_factorial();
    test_call_stack_overflow();
    test_ret_without_call();
    test_load_args();
    test_store();
    test_ret_restores_frame();
    test_load_invalid();
    test_store_into_popped_slot();
    printf("\n%s (%d failures)\n", failures ? "FAILED" : "ALL PASSED", failures);
    return failures ? 1 : 0;
}