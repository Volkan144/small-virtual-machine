#include <stdio.h>
#include "vm.h"
#include "opcodes.h"

void vm_init(VM *vm, const int32_t *code, size_t code_len) {
    vm->code = code;
    vm->code_len = code_len;
    vm->pc = 0;
    vm->sp = 0;
    vm->csp = 0;
}

static VMResult fetch_operand(VM *vm, int32_t *out){
    if (vm->pc >= vm->code_len){
        return VM_ERR_PC_OUT_OF_BOUNDS;
    }
    *out = vm->code[vm->pc++];
    return VM_OK;
}

static VMResult push(VM *vm, int32_t value) {
    if (vm->sp >= STACK_SIZE){
        return VM_ERR_STACK_OVERFLOW;
    }
    vm->stack[vm->sp] = value;
    vm->sp +=1;
    return VM_OK;
}
static VMResult call_push(VM *vm, Frame frame) {
    if (vm->csp >= CALL_STACK_SIZE){
        return VM_ERR_CALL_STACK_OVERFLOW;
    }
    vm->callstack[vm->csp] = frame;
    vm->csp +=1;
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

static VMResult call_pop(VM *vm, Frame *out) {
    if (vm->csp <= 0){
        return VM_ERR_CALL_STACK_UNDERFLOW;
    }
    vm->csp -=1;
    *out = vm->callstack[vm->csp];
    return VM_OK;
}

static VMResult read_target(VM *vm, size_t *dest){
    int32_t destination;
    VMResult r = fetch_operand(vm, &destination);
    if (r != VM_OK) return r;
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
            int32_t value;
            VMResult r = fetch_operand(vm, &value);
            if (r != VM_OK) return r;
            r = push(vm, value);
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

        case OP_CALL:{
            size_t dest;
            int32_t count;
            Frame frame;
            VMResult r = read_target(vm, &dest);
            if (r != VM_OK) return r;
            r= fetch_operand(vm, &count);
            if (r != VM_OK) return r;
            if (count < 0 || (size_t)count > vm->sp) return VM_ERR_STACK_UNDERFLOW;
            frame.return_addr = vm->pc;
            frame.saved_fp = vm->fp;
            r = call_push(vm, frame);
            if (r != VM_OK) return r;
            vm->pc = dest;
            vm->fp = vm->sp -count;
            break;

        }

        case OP_RET:{
            Frame frame;
            int32_t value;
            VMResult r = call_pop(vm, &frame);
            if (r != VM_OK) return r;
            if (vm->sp <= vm->fp) return VM_ERR_STACK_UNDERFLOW;
            r = pop(vm, &value);
            if (r != VM_OK) return r;
            vm->sp = vm->fp;
            r = push(vm, value);
            if (r != VM_OK) return r;
            vm->pc = frame.return_addr;
            vm->fp = frame.saved_fp;
            break;

        }
        
        case OP_LOAD:{
            int32_t index;
            VMResult r= fetch_operand(vm, &index);
            if (r != VM_OK) return r;
            if (index < 0 || vm->fp + (size_t)index >= vm->sp) return VM_ERR_INVALID_LOCAL;
            int32_t value = vm->stack[vm->fp+index];
            r = push(vm, value);
            if (r != VM_OK) return r;
            break;
        }

        case OP_STORE:{
            int32_t index;
            VMResult r= fetch_operand(vm, &index);
            if (r != VM_OK) return r;
            int32_t value;
            r = pop(vm, &value);
            if (r != VM_OK) return r;
            if (index < 0 || vm->fp + (size_t)index >= vm->sp) return VM_ERR_INVALID_LOCAL;
            vm->stack[vm->fp+index] = value;
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
