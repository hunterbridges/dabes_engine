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

typedef struct DrawBufferLayer {
    int z_index;
    DArray *textures;
    Hashmap *texture_buffers;
} DrawBufferLayer;

DrawBufferLayer *DrawBufferLayer_create(int z_index);
void DrawBufferLayer_buffer(DrawBufferLayer *layer, GfxTexture *texture,
                            int num_points, int num_attrs, VVector4 vectors[]);
void DrawBufferLayer_destroy(DrawBufferLayer *layer);
void DrawBufferLayer_draw(DrawBufferLayer *layer);

typedef struct DrawBuffer {
    List *layers;
} DrawBuffer;

DrawBuffer *DrawBuffer_create();
void DrawBuffer_destroy(DrawBuffer *buffer);
void DrawBuffer_buffer(DrawBuffer *buffer, GfxTexture *texture, int z_index,
                       int num_points, int num_attrs, VVector4 vectors[]);
void DrawBuffer_empty(DrawBuffer *buffer);
void DrawBuffer_draw(DrawBuffer *buffer);

#endif
