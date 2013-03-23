#ifndef __util_h
#define __util_h
#include "prefix.h"
#define sign(N) (N > 0) - (N < 0)
#define MIN(A, B) (A < B ? A : B)
#define MAX(A, B) (A > B ? A : B)

int read_text_file(char *filename, GLchar **out, GLint *size);

#endif