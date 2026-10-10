#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

/* Reads a binary program file into a newly allocated array.
 * Returns NULL on error. The caller must free() the result. */
static int32_t *load_program(const char *path, size_t *len) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        perror(path);
        return NULL;
    }

    /* Determine file size: jump to the end, ask for the position, jump back. */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size % sizeof(int32_t) != 0) {
        fprintf(stderr, "%s: not a valid program file\n", path);
        fclose(f);
        return NULL;
    }

    size_t count = (size_t)size / sizeof(int32_t);
    int32_t *code = malloc((size_t)size);
    if (code == NULL) {
        fprintf(stderr, "out of memory\n");
        fclose(f);
        return NULL;
    }

    /* fread returns how many words it actually read. */
    if (fread(code, sizeof(int32_t), count, f) != count) {
        fprintf(stderr, "%s: read error\n", path);
        free(code);
        fclose(f);
        return NULL;
    }

    fclose(f);
    *len = count;
    return code;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    size_t len;
    int32_t *code = load_program(argv[1], &len);
    if (code == NULL)
        return 1;

    VM vm;
    vm_init(&vm, code, len);
    VMResult r = vm_run(&vm);
    free(code);

    if (r != VM_OK) {
        fprintf(stderr, "VM error at pc=%zu: %s\n", vm.pc, vm_result_str(r));
        return 1;
    }
    return 0;
}