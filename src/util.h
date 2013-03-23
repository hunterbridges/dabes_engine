#ifndef __util_h
#define __util_h
#include "prefix.h"
#define sign(N) (N > 0) - (N < 0)

#ifndef DABES_IOS
#define MIN(A, B) (A < B ? A : B)
#define MAX(A, B) (A > B ? A : B)
#endif

FILE *load_resource(char *filename);
int read_text_file(char *filename, GLchar **out, GLint *size);
#ifdef DABES_IOS
int read_file_data(char *filename, unsigned long int **data, GLint *size);
#endif

#endif