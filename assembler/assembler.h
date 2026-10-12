#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#define MAX_CODE 65536

int assemble(FILE *in, int32_t *code, size_t *len);

#endif