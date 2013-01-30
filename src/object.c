#include "prefix.h"

void Object_destroy(void *self)
{
    assert(self != NULL);
    Object *obj = self;

    if (obj) {
        if (obj->description) free(obj->description);
        free(obj);
    }
}

void Object_describe(void *self)
{
    assert(self != NULL);
    Object *obj = self;
    printf("%s.\n", obj->description);
}

int Object_init(void *self)
{
    assert(self != NULL);
    // do nothing really
    return 1;
}

void Object_calc_physics(void *self, void *engine, int ticks)
{
    assert(self != NULL);
}

void Object_render(void *self, void *engine) {
    assert(self != NULL);
}

void *Object_new(size_t size, Object proto, char *description)
{
    assert(description != NULL);

    // setup the default functions in case they aren't set
    if (!proto.init) proto.init = Object_init;
    if (!proto.describe) proto.describe = Object_describe;
    if (!proto.destroy) proto.destroy = Object_destroy;
    if (!proto.calc_physics) proto.calc_physics = Object_calc_physics;
    if (!proto.render) proto.render = Object_render;

    // this seems weird, but we can make a struct of one size,
    // then point a different pointer at it to "cast" it
    Object *el = calloc(1, size);
    assert(el != NULL);
    *el = proto;

    // copy the description over
    el->description = strdup(description);

    // initialize it with whatever init we were given
    if (!el->init(el)) {
        // looks like it didn't initialize properly
        el->destroy(el);
        return NULL;
    } else {
        // all done, we made an object of any type
        return el;
    }
}

