#ifndef __util_h
#define __util_h
#include "prefix.h"
#define sign(N) (N > 0) - (N < 0)

#ifndef MIN
#define MIN(A, B) (A < B ? A : B)
#endif
#ifndef MAX
#define MAX(A, B) (A > B ? A : B)
#endif

#define fequal(a,b) (fabs((a) - (b)) < FLT_EPSILON)
#define streq(A, B) (strcmp((const char *)A, (const char *)B) == 0)

const char *resource_path(const char *filename);
FILE *load_resource(char *filename);
int read_text_file(char *filename, GLchar **out, GLint *size);
#ifdef DABES_IOS
int read_file_data(char *filename, unsigned long int **data, GLint *size);
#endif

#endif
