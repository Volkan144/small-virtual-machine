#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

void vm_init(VM *vm, const int32_t *code, size_t code_len) {
    vm->code = code;
    vm->code_len = code_len;
    vm->pc = 0;
    vm->sp = 0;
}

static VMResult push(VM *vm, int32_t value) {
    if (vm->sp >= STACK_SIZE){
        return VM_ERR_STACK_OVERFLOW;
    }
    vm->stack[vm->sp] = value;
    vm->sp +=1;
    return VM_OK;
}

static VMResult pop(VM *vm, int32_t *out) {
    if (vm->sp <= 0){
        return VM_ERR_STACK_UNDERFLOW;
    }
    vm->sp -=1;
    *out = vm->stack[vm->sp];
    return VM_OK;
}

static VMResult read_target(VM *vm, size_t *dest){
    int32_t destination = vm->code[vm->pc++];
    if(destination < 0 || (size_t)destination >= vm->code_len) return VM_ERR_PC_OUT_OF_BOUNDS;
    *dest = destination;
    return VM_OK;
}

VMResult vm_run(VM *vm) {

    for (;;) {
        if (vm->pc >= vm->code_len)
            return VM_ERR_PC_OUT_OF_BOUNDS;

        /* fetch */
        int32_t op = vm->code[vm->pc++];

        /* decode + execute */
        switch (op) {
        case OP_HALT:
            return VM_OK;
        
        case OP_PUSH:{
            int32_t value = vm->code[vm->pc++];
            VMResult r = push(vm, value);
            if (r != VM_OK)
                return r;
            break;
        }
        
        case OP_POP:{
            int32_t a;                 
            VMResult r = pop(vm, &a); 
            if (r != VM_OK)
                return r; 
            break;
        }
        
        case OP_ADD:{
            int32_t a;
            int32_t b;
            VMResult r = pop(vm, &b);
            if (r != VM_OK) return r;

            r = pop(vm, &a);
            if (r != VM_OK) return r;

            r = push(vm, a + b);
            if (r != VM_OK) return r;
            break;
        }
        case OP_SUB:{
            int32_t a; 
            int32_t b;
            VMResult r = pop(vm, &b);
            if(r != VM_OK) return r;

            r = pop(vm, &a);
            if(r != VM_OK) return r;

            r  = push(vm, a-b);
            if(r != VM_OK) return r;
            break;

        }

        case OP_MUL:{
            int32_t a;
            int32_t b;
            VMResult r = pop(vm, &b);
            if (r != VM_OK) return r;

            r = pop(vm, &a);
            if (r != VM_OK) return r;

            r = push(vm, a * b);
            if (r != VM_OK) return r;
            break;
        }

        case OP_PRINT: {
            if (vm->sp <= 0){
                return VM_ERR_STACK_UNDERFLOW;
    }
            int32_t value = vm->stack[vm->sp -1] ;
            printf("%d\n", value);

            break;
        }

        case OP_DUP:{
            if (vm->sp <= 0){
                return VM_ERR_STACK_UNDERFLOW;
    }
            int32_t value = vm->stack[vm->sp -1] ;
            VMResult r = push(vm, value);
            if (r!=VM_OK) return r;
            break;

        }
        case OP_JMP:{
            size_t dest;
            VMResult r = read_target(vm, &dest);
            if (r != VM_OK) return r;
            vm->pc = dest;
            break;
        }
        case OP_JZ:{
    
            size_t dest;
            int32_t value;
            VMResult r = pop(vm, &value);
            if (r != VM_OK) return r;
            r = read_target(vm, &dest);
            if (r != VM_OK) return r;
            if (value == 0){ 
                vm->pc = dest;
            }   
            break;
        }
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
