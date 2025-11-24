#include "data.h"

#include <stdio.h>
#include <stdlib.h>

/* FUNCS */

char* engLoadTextFile(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("failed to open text file at \"%s\"!\n", path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = malloc(size + 1);
    if (!buffer) {
        printf("allocation failure in loading text file \"%s\"!\n", path);
        fclose(file);
        return 0;
    }

    fread(buffer, 1, size, file);
    fclose(file);

    buffer[size] = '\0';

    return buffer;
}

char* engLoadDataFile(const char* path, size_t* out_size) {
    FILE* file = fopen(path, "rb");
    if (!file) {
        printf("failed to open data file at \"%s\"!\n", path);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char* buffer = malloc(size);
    if (!buffer) {
        printf("allocation failure in loading data file \"%s\"!\n", path);
        fclose(file);
        return 0;
    }

    fread(buffer, 1, size, file);
    fclose(file);

    if (out_size)
        *out_size = size;

    return buffer;
}
