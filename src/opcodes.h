#ifndef OPCODES_H
#define OPCODES_H

/* Instruction set of the VM. */
typedef enum {
    OP_HALT = 0,  /* stop execution                          */
    OP_PUSH,      /* PUSH <value> : push value onto stack    */
    OP_POP,       /* discard top of stack                    */
    OP_ADD,       /* a b -> a+b                              */
    OP_SUB,       /* a b -> a-b                              */
    OP_MUL,       /* a b -> a*b                              */
    OP_DUP,       /* a -> a a                                */
    OP_JMP,       /* JMP <addr>   : pc = addr                */
    OP_JZ,        /* JZ <addr>    : pop; if 0 then pc = addr */
    OP_PRINT,     /* print top of stack (without popping)    */
    /* v2: OP_CALL, OP_RET, OP_LOAD, OP_STORE, ... */
    OP_CALL,
    OP_RET,
    OP_COUNT
} Opcode;

#endif
