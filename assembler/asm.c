#include "assembler.h"

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
    static int32_t code[MAX_CODE];
    size_t len;

    int result = assemble(in, code, &len);
    fclose(in);
    if (result != 0) return 1;
    
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