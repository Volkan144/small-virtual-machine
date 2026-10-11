#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "opcodes.h"

#define MAX_CODE 65536
#define MAX_LINE 256

static int32_t code[MAX_CODE];
static size_t len = 0;

typedef struct{
    const char *name;
    int32_t opcode;
    int nargs;
} Instr;

#define MAX_LABELS 256
#define MAX_LABEL_LEN 32

typedef struct {
    char name[MAX_LABEL_LEN];
    int32_t addr;
} Label;

static Label labels[MAX_LABELS];
static size_t num_labels = 0;


static const Instr instrs[] = {
    { "halt",  OP_HALT,  0 },
    { "push",  OP_PUSH,  1 },
    { "pop",   OP_POP,   0 },
    { "add",   OP_ADD,   0 },
    { "sub",   OP_SUB,   0 },
    { "mul",   OP_MUL,   0 },
    { "dup",   OP_DUP,   0 },
    { "jmp",   OP_JMP,   1 },
    { "jz",    OP_JZ,    1 },
    { "print", OP_PRINT, 0 },
    { "call",  OP_CALL,  2 },
    { "ret",   OP_RET,   0 },
    { "load",  OP_LOAD,  1 },
    { "store", OP_STORE, 1 },
};
#define NUM_INSTRS (sizeof instrs / sizeof instrs[0])

static const Instr *find_instr(const char *name){
    for(size_t i=0; i<NUM_INSTRS; i++){
        if (strcmp(name, instrs[i].name) == 0) {
            return &instrs[i];
            
        }
    }
    return NULL;
}

#define NUM_INSTRS (sizeof instrs / sizeof instrs[0])


static const Label *find_label(const char *name){
    for(size_t i = 0; i < num_labels; i++){
        if(strcmp(name, labels[i].name) == 0){
            return &labels[i];
        }
    }
    return NULL;

}

static int add_label(const char *name, int32_t addr, int lineno){
    if(find_label(name) != NULL) {
        fprintf(stderr, "line %d: duplicate label '%s'\n", lineno, name);
        return 1;
    }
    if(num_labels >= MAX_LABELS){
        fprintf(stderr, "line %d: maximum labelamount %d reached  \n", lineno, MAX_LABELS);
        return 1;
    } 
    
    if(strlen(name)>= MAX_LABEL_LEN){
        fprintf(stderr, "line %d: label has too many characters. %zu>%d  \n",lineno, strlen(name), MAX_LABEL_LEN);
        return 1;
    }
    
    strcpy(labels[num_labels].name, name);
    labels[num_labels].addr = addr;
    num_labels++;
    return 0;

}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <input.asm> <output.bin>\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");  //open assembly-file        
    if (in == NULL) {       
        perror(argv[1]);
        return 1;
    }

    char line[MAX_LINE];
    int lineno = 0;

    /* fgets reads one line per call, returns NULL at end of file */
    for (int pass = 1; pass <= 2; pass++){
        fseek(in, 0, SEEK_SET);
        len = 0;
        lineno = 0;
        while (fgets(line, sizeof line, in) != NULL) {
            lineno++;
            char *comment = strchr(line, ';');
            if (comment != NULL)
                *comment = '\0';

            char *mnemonic = strtok(line, " \t\r\n");
            if (mnemonic == NULL) continue;
            size_t n = strlen(mnemonic);
            if (mnemonic[n - 1] == ':') {
                mnemonic[n - 1] = '\0';
                if(pass== 1){
                    if (add_label(mnemonic, len, lineno) != 0) return 1;
                }
                mnemonic = strtok(NULL, " \t\r\n");
                if(mnemonic == NULL) continue; 
            }

            const Instr *ins = find_instr(mnemonic);
            if (ins == NULL) {
                fprintf(stderr, "line %d: unknown instruction '%s'\n", lineno, mnemonic);
                return 1;
            }
            code[len++] = ins->opcode;
            
            for(int32_t i = 0; i<ins->nargs; i++){
            char *arg = strtok(NULL, " \t\r\n");
            if (arg == NULL) {
                fprintf(stderr, "line %d: missing parameter '%s'\n", lineno, mnemonic);
                return 1;
            }
            
            char *end;
            int value = strtol(arg, &end, 10);
            if (*end != '\0'){
                if(pass == 1){
                    value = 0;
                }
                else {
                    const Label *label = find_label(arg);
                    if(label == NULL) {
                        fprintf(stderr, "line %d: unknown label '%s'\n", lineno, arg);
                        return 1;
                    }
                    value = label->addr;
                }
            }
            code[len++] = value;   
            }
        }
    }
    fclose(in);

    FILE *out = fopen(argv[2], "wb");        
    if (out == NULL) {
        perror(argv[2]);
        return 1;
    }
    if (fwrite(code, sizeof(int32_t), len, out) != len) {
        fprintf(stderr, "%s: write error\n", argv[2]);
        fclose(out);
        return 1;
    }
    fclose(out);
    return 0;
}