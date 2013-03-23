#include "util.h"

int read_text_file(char *filename, GLchar **out, GLint *size) {
#ifdef DABES_IOS
    char abs_path[512];
    strcpy(abs_path, bundlePath__);
    strcat(abs_path, filename);
#else
    char *abs_path = filename;
#endif
    FILE *file = fopen(abs_path, "r");
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
