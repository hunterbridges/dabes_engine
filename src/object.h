#ifndef _object_h
#define _object_h
#include "prefix.h"

typedef struct {
    char *description;
    int (*init)(void *self);
    void (*describe)(void *self);
    void (*destroy)(void *self);
    void (*render)(void *self, void *engine);
} Object;

int Object_init(void *self);
void Object_destroy(void *self);
void Object_describe(void *self);
void *Object_new(size_t size, Object proto, char *description);
void Object_render(void *self, void *engine);

#define NEW(T, N) Object_new(sizeof(T), T##Proto, N)
#define _(N) proto.N
#define m_(N, M, ...) N->M(N, ##__VA_ARGS__)

#endif
