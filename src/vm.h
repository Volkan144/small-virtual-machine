#ifndef VM_H
#define VM_H

#include <stddef.h>
#include <stdint.h>

#define STACK_SIZE 256
#define CALL_STACK_SIZE 256

typedef enum {
    VM_OK = 0,
    VM_ERR_STACK_OVERFLOW,
    VM_ERR_STACK_UNDERFLOW,
    VM_ERR_UNKNOWN_OPCODE,
    VM_ERR_PC_OUT_OF_BOUNDS,
    VM_ERR_CALL_STACK_OVERFLOW,
    VM_ERR_CALL_STACK_UNDERFLOW,
    VM_ERR_INVALID_LOCAL
} VMResult;

typedef struct {
    size_t return_addr;
    size_t saved_fp;
} Frame;

typedef struct {
    const int32_t *code;   /* bytecode                         */
    size_t code_len;       /* number of words in the program   */
    size_t pc;             /* program counter                  */

    int32_t stack[STACK_SIZE];
    Frame callstack[CALL_STACK_SIZE];
    size_t sp;             /* index of the next free slot      */
    size_t csp;
    size_t fp;
} VM;

void        vm_init(VM *vm, const int32_t *code, size_t code_len);
VMResult    vm_run(VM *vm);
const char *vm_result_str(VMResult r);

#endif
