#ifndef __util_h
#define __util_h
#include "prefix.h"
#define sign(N) (N > 0) - (N < 0)

int read_text_file(char *filename, GLchar **out, GLint *size);

#endif
