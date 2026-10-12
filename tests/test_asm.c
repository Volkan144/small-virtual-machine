#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "opcodes.h"

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("FAIL: %s\n", msg); failures++; } \
    else         { printf("ok:   %s\n", msg); } \
} while (0)

static int32_t code[MAX_CODE];
static size_t len;

/* Assembles a source string, returns the result of assemble(). */
static int run_asm(const char *src) {
    FILE *f = fmemopen((void *)src, strlen(src), "r");
    int r = assemble(f, code, &len);
    fclose(f);
    return r;
}

/* Compares the assembled output with an expected word sequence. */
static int code_equals(const int32_t *expected, size_t n) {
    if (len != n)
        return 0;
    for (size_t i = 0; i < n; i++)
        if (code[i] != expected[i])
            return 0;
    return 1;
}

#define EXPECT_CODE(expected, msg) \
    CHECK(code_equals(expected, sizeof expected / sizeof expected[0]), msg)

/* ---------- valid programs ---------- */

static void test_push(void) {
    CHECK(run_asm("push 5\nhalt\n") == 0, "push assembles without error");
    CHECK(len == 3, "push 5 / halt produces 3 words");
    CHECK(code[0] == OP_PUSH && code[1] == 5 && code[2] == OP_HALT,
          "push 5 / halt encodes as 1, 5, 0");
}

static void test_empty_input(void) {
    CHECK(run_asm("") == 0, "empty input assembles without error");
    CHECK(len == 0, "empty input produces no code");
}

static void test_comments_and_blank_lines(void) {
    const int32_t expected[] = { OP_PUSH, 1, OP_HALT };
    CHECK(run_asm("; full-line comment\n\n   \npush 1 ; trailing comment\nhalt\n") == 0,
          "comments and blank lines assemble without error");
    EXPECT_CODE(expected, "comments and blank lines are ignored");
}

static void test_negative_number(void) {
    const int32_t expected[] = { OP_PUSH, -7, OP_HALT };
    CHECK(run_asm("push -7\nhalt\n") == 0, "negative number assembles without error");
    EXPECT_CODE(expected, "negative numbers are encoded correctly");
}

static void test_crlf_line_endings(void) {
    const int32_t expected[] = { OP_PUSH, 5, OP_HALT };
    CHECK(run_asm("push 5\r\nhalt\r\n") == 0, "CRLF input assembles without error");
    EXPECT_CODE(expected, "Windows line endings are handled");
}

static void test_label_backward(void) {
    const int32_t expected[] = { OP_PUSH, 1, OP_JMP, 0 };
    CHECK(run_asm("loop: push 1\njmp loop\n") == 0, "backward label assembles without error");
    EXPECT_CODE(expected, "backward reference resolves to address 0");
}

static void test_label_forward(void) {
    /* 0: jmp end   2: push 99   4: end: halt */
    const int32_t expected[] = { OP_JMP, 4, OP_PUSH, 99, OP_HALT };
    CHECK(run_asm("jmp end\npush 99\nend: halt\n") == 0, "forward label assembles without error");
    EXPECT_CODE(expected, "forward reference resolves in the second pass");
}

static void test_label_on_own_line(void) {
    const int32_t expected[] = { OP_PUSH, 2, OP_JMP, 0 };
    CHECK(run_asm("start:\n    push 2\n    jmp start\n") == 0,
          "label on its own line assembles without error");
    EXPECT_CODE(expected, "label on its own line marks the next instruction");
}

static void test_call_with_label(void) {
    /* 0: call f 1   3: halt   4: f: ret */
    const int32_t expected[] = { OP_CALL, 4, 1, OP_HALT, OP_RET };
    CHECK(run_asm("call f 1\nhalt\nf: ret\n") == 0, "call with label assembles without error");
    EXPECT_CODE(expected, "call takes a label and an argument count");
}

static void test_factorial_program(void) {
    const char *src =
        "        push 5\n"
        "        call fact 1\n"
        "        print\n"
        "        halt\n"
        "\n"
        "fact:   dup\n"
        "        jz base\n"
        "        dup\n"
        "        push 1\n"
        "        sub\n"
        "        call fact 1\n"
        "        mul\n"
        "        ret\n"
        "\n"
        "base:   push 1\n"
        "        ret\n";
    const int32_t expected[] = {
        OP_PUSH, 5, OP_CALL, 7, 1, OP_PRINT, OP_HALT,
        OP_DUP, OP_JZ, 19, OP_DUP, OP_PUSH, 1, OP_SUB, OP_CALL, 7, 1, OP_MUL, OP_RET,
        OP_PUSH, 1, OP_RET,
    };
    CHECK(run_asm(src) == 0, "factorial program assembles without error");
    EXPECT_CODE(expected, "factorial program matches the hand-assembled bytecode");
}

static void test_labels_reset_between_runs(void) {
    CHECK(run_asm("a: halt\n") == 0, "first run with label 'a' succeeds");
    CHECK(run_asm("a: halt\n") == 0, "second run with label 'a' succeeds (labels are reset)");
}

/* ---------- errors ---------- */

static void test_unknown_instruction(void) {
    CHECK(run_asm("pusj 5\n") != 0, "unknown instruction is rejected");
}

static void test_missing_argument(void) {
    CHECK(run_asm("push\n") != 0, "missing argument is rejected");
}

static void test_missing_second_argument(void) {
    CHECK(run_asm("f: call f\n") != 0, "call with only one argument is rejected");
}

static void test_unknown_label(void) {
    CHECK(run_asm("jmp nowhere\n") != 0, "unknown label is rejected");
}

static void test_invalid_number(void) {
    CHECK(run_asm("push 5x\n") != 0, "invalid number is rejected");
}

static void test_duplicate_label(void) {
    CHECK(run_asm("a: halt\na: halt\n") != 0, "duplicate label is rejected");
}

static void test_label_too_long(void) {
    CHECK(run_asm("a_label_name_that_is_way_too_long_for_the_table: halt\n") != 0,
          "label longer than 31 characters is rejected");
}

int main(void) {
    test_push();
    test_empty_input();
    test_comments_and_blank_lines();
    test_negative_number();
    test_crlf_line_endings();
    test_label_backward();
    test_label_forward();
    test_label_on_own_line();
    test_call_with_label();
    test_factorial_program();
    test_labels_reset_between_runs();

    printf("\n-- error cases (the messages above them are expected) --\n");
    test_unknown_instruction();
    test_missing_argument();
    test_missing_second_argument();
    test_unknown_label();
    test_invalid_number();
    test_duplicate_label();
    test_label_too_long();

    printf("\n%s (%d failures)\n", failures ? "FAILED" : "ALL PASSED", failures);
    return failures ? 1 : 0;
}