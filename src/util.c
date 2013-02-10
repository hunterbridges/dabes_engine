#include "util.h"

int read_text_file(char *filename, GLchar **out, GLuint *size) {
    FILE *file = fopen(filename, "r");
    check_mem(file);

    fseek(file, 0L, SEEK_END);
    unsigned int sz = ftell(file);
    fseek(file, 0L, SEEK_SET);

    GLchar *output = malloc((*size + 1) * sizeof(GLchar));
    check_mem(output);

    fread(output, sizeof(GLchar), *size, file);
    fclose(file);
    output[*size] = '\0';

    *out = output;
    *size = sz;

    return 1;
error:
    if (file != NULL) fclose(file);
    if (out != NULL) free(output);
    return 0;
}
