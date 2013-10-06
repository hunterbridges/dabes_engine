#ifndef __draw_buffer_h
#define __draw_buffer_h
#include <lcthw/bstrlib.h>
#include <lcthw/darray.h>
#include <lcthw/hashmap.h>
#include <lcthw/hashmap_algos.h>
#include <lcthw/list.h>
#include "../prefix.h"
#include "../math/vmatrix.h"
#include "graphics.h"

typedef struct DrawBufferShape {
    int num_points;
    int num_vectors;
    VMatrix vectors[];
} DrawBufferShape;

typedef struct DrawBufferTexture {
    GfxTexture *texture;
    bstring key;
} DrawBufferTexture;

typedef struct DrawBuffer {
    DArray *textures;
    Hashmap *texture_buffers;
    short int populated;
} DrawBuffer;

DrawBuffer *DrawBuffer_create();
void DrawBuffer_destroy(DrawBuffer *buffer);
void DrawBuffer_buffer(DrawBuffer *buffer, GfxTexture *texture,
                       int num_points, int num_attrs, VVector4 vectors[]);
void DrawBuffer_empty(DrawBuffer *buffer);
void DrawBuffer_draw(DrawBuffer *buffer, Graphics *graphics);

#endif
