/* v3: Assembler
 * Reads a .asm file (see programs/) and writes bytecode.
 * Plan:
 *   pass 1: collect labels (name -> address)
 *   pass 2: translate mnemonics into opcodes, resolve labels
 */
#include <stdio.h>
#include "opcodes.h"

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <input.asm> <output.bin>\n", argv[0]);
        return 1;
    }
    fprintf(stderr, "assembler not implemented yet (v3)\n");
    return 1;
}
