#include "util.h"

FILE *load_resource(char *filename) {
#ifdef DABES_IOS
    // TODO: Make This Better
    char abs_path[512];
    strcpy(abs_path, bundlePath__);
    strcat(abs_path, filename);
#else
    char *abs_path = filename;
#endif
    FILE *file = fopen(abs_path, "r");
    return file;
}

int read_text_file(char *filename, GLchar **out, GLint *size) {
    FILE *file = load_resource(filename);
    check(file != NULL, "Failed to open %s", filename);

    fseek(file, 0, SEEK_END);
    unsigned int sz = ftell(file);
    rewind(file);

    GLchar *output = malloc(sz * sizeof(char));
    check_mem(output);

    fread(output, 1, sz, file);
    fclose(file);
    output[sz] = '\0';

    *out = output;
    *size = sz;

    return 1;
error:
    if (file != NULL) fclose(file);
    if (out != NULL) free(output);
    return 0;
}

int read_file_data(char *filename, unsigned long int **data, GLint *size) {
    unsigned long int *output = NULL;
    FILE *file = load_resource(filename);
    check(file != NULL, "Failed to open %s", filename);

    fseek(file, 0, SEEK_END);
    unsigned int sz = ftell(file);
    rewind(file);

    output = malloc(sz * sizeof(unsigned long int));
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
