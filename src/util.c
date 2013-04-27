#include "util.h"

#ifndef DABES_IOS
const char *resource_path(const char *filename) {
  return filename;
}

FILE *load_resource(char *filename) {
    FILE *file = fopen(filename, "r");
    return file;
}
#endif

int read_text_file(char *filename, GLchar **out, GLint *size) {
    GLchar *output = NULL;
    FILE *file = load_resource(filename);
    check(file != NULL, "Failed to open %s", filename);

    fseek(file, 0, SEEK_END);
    unsigned int sz = ftell(file);
    rewind(file);

    output = malloc(sz * sizeof(char));
    check_mem(output);

    fread(output, 1, sz, file);
    fclose(file);
    output[sz] = '\0';

    *out = output;
    *size = sz;

    return 1;
error:
    if (file != NULL) fclose(file);
    if (output != NULL) free(output);
    return 0;
}

int read_file_data(char *filename, unsigned long int **data, GLint *size) {
    unsigned long int *output = NULL;
    FILE *file = load_resource(filename);
    check(file != NULL, "Failed to open %s", filename);

    fseek(file, 0, SEEK_END);
    unsigned int sz = ftell(file);
    rewind(file);

    output = malloc(sz * sizeof(unsigned long int) + 1);
    check_mem(output);

    fread(output, 1, sz, file);
    fclose(file);
    output[sz] = '\0';

    *data = output;
    *size = sz;

    return 1;
error:
    if (file != NULL) fclose(file);
    if (output != NULL) free(output);
    return 0;
}
