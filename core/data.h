#pragma once

#include <stdio.h>

/* FUNCS */

char* engLoadTextFile(const char* path);
char* engLoadDataFile(const char* path, size_t* out_size);
