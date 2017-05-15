#include <stdio.h>
#include <stdint.h>

#include "arquivos.h"

FILE* in;
FILE* out;

int arq_open(char const* input, char const* output) {
    if(!(in = fopen(input, "r")))
        return 0;
    if(!(out = fopen(output, "w")))
        return 0;
    return 1;
}

void arq_close() {
    fclose(in);
    fclose(out);
}

size_t arq_read(char* buf) {
    size_t aux = fread(buf, sizeof(char), INT16_MAX, in);
    /*if(feof(in) && aux < 0)
        aux = 0;*/
    return aux;
}

int arq_read_end() {
    return feof(in);
}

int arq_write(char const* buf, int length) {
    if(fwrite(buf, sizeof(char), length, out) == length)
        return 1;
    else
        return 0;
}