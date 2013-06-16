#include "draw_buffer.h"

DrawBufferLayer *DrawBufferLayer_create(int z_index) {
    DrawBufferLayer *layer = calloc(1, sizeof(DrawBufferLayer));
    check(layer != NULL, "Couldn't create draw buffer layer");
    
    layer->z_index = z_index;
    layer->texture_buffers = Hashmap_create(NULL, Hashmap_fnv1a_hash);
    layer->textures = DArray_create(sizeof(DrawBufferTexture *), 10);
    
    return layer;
error:
    return NULL;
}

void DrawBufferLayer_buffer(DrawBufferLayer *layer, GfxTexture *texture,
        int num_points, int num_attrs, VVector4 vectors[]) {
    char key[32];
    int result = sprintf(key, "%p", texture);
    assert(result < 31);
    
    bstring bkey = bfromcstr(key);
    
    int i = 0;
    int found = 0;
    DArray *shapes = NULL;
    for (i = 0; i < DArray_count(layer->textures); i++) {
        DrawBufferTexture *buftex = DArray_get(layer->textures, i);
        if (!streq(buftex->key, bkey)) continue;
            
        found = 1;
        shapes = Hashmap_get(layer->texture_buffers, bkey);
        bdestroy(bkey);
        break;
    }
    
    if (found == 0) {
        DrawBufferTexture *buftex = calloc(1, sizeof(DrawBufferTexture));
        check(buftex != NULL, "Couldn't create draw buffer texture");
        buftex->texture = texture;
        buftex->key = bkey;
        DArray_push(layer->textures, buftex);
        
        shapes = DArray_create(sizeof(DrawBufferShape *), 24);
        Hashmap_set(layer->texture_buffers, bkey, shapes);
    }
    
    int num_vectors = num_attrs * num_points;
    DrawBufferShape *shape = calloc(1, sizeof(DrawBufferShape) +
                                    sizeof(VVector4) * num_vectors);
    check(shape != NULL, "Couldn't create draw buffer shape");
    
    shape->num_points = num_points;
    shape->num_vectors = num_vectors;
    memcpy(shape->vectors, vectors, sizeof(VVector4) * num_vectors);
    
    DArray_push(shapes, shape);
    
    return;
error:
    return;
}

void DrawBufferLayer_destroy(DrawBufferLayer *layer) {
    check(layer != NULL, "No Draw buffer layer to destroy");
    DArray_clear_destroy(layer->textures);
    Hashmap_destroy(layer->texture_buffers,
                    (Hashmap_destroy_func)DArray_clear_destroy);
    free(layer);
    return;
error:
    return;
}

void DrawBufferLayer_draw(DrawBufferLayer *layer) {
    int i = 0;
    for (i = 0; i < DArray_count(layer->textures); i++) {
        DrawBufferTexture *buftex = DArray_get(layer->textures, i);
        DArray *texshapes = Hashmap_get(layer->texture_buffers, buftex->key);
        int j = 0;
        int num_points = 0;
        int num_vectors = 0;
        for (j = 0; j < DArray_count(texshapes); j++) {
            DrawBufferShape *shape = DArray_get(texshapes, j);
            num_points += shape->num_points;
            num_vectors += shape->num_vectors;
        }
        
        // Copy the shapes into an inline buffer
        VVector4 vectors[num_vectors];
        VVector4 *v_head = vectors;
        for (j = 0; j < DArray_count(texshapes); j++) {
            DrawBufferShape *shape = DArray_get(texshapes, j);
            memcpy(v_head, shape->vectors,
                   sizeof(VVector4) * shape->num_vectors);
            v_head += shape->num_vectors;
        }
        
        glBindTexture(GL_TEXTURE_2D,
                      buftex->texture ? buftex->texture->gl_tex : 0);
        glBufferData(GL_ARRAY_BUFFER, num_vectors * sizeof(VVector4), vectors,
                GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, num_points);
    }
}

////////////////////////////////////////////////////////////////////////////////

DrawBuffer *DrawBuffer_create() {
    DrawBuffer *buffer = calloc(1, sizeof(DrawBuffer));
    check(buffer != NULL, "Couldn't create Draw Buffer");
    
    return buffer;
error:
    return NULL;
}

void DrawBuffer_destroy(DrawBuffer *buffer) {
    check(buffer != NULL, "No Draw Buffer to destroy");
    DrawBuffer_empty(buffer);
    free(buffer);
    return;
error:
    return;
}

void DrawBuffer_buffer(DrawBuffer *buffer, GfxTexture *texture, int z_index,
                       int num_points, int num_attrs, VVector4 vectors[]) {
    assert(buffer != NULL);
    if (!buffer->layers) {
        buffer->layers = List_create();
    }
    
    DrawBufferLayer *layer = NULL;
    if (buffer->layers->first) {
        LIST_FOREACH(buffer->layers, first, next, current) {
            DrawBufferLayer *this_layer = current->value;
            DrawBufferLayer *prev_layer = NULL;
            if (current->prev) prev_layer = current->prev->value;
            
            if (this_layer->z_index == z_index) {
                layer = this_layer;
                break;
            } else if (this_layer->z_index > z_index) {
                layer = DrawBufferLayer_create(z_index);
                if (prev_layer) {
                    ListNode *insert = calloc(1, sizeof(ListNode));
                    
                    insert->value = layer;
                    insert->prev = current->prev;
                    insert->next = current;
                    current->prev->next = insert;
                    current->prev = insert;
                } else {
                    List_unshift(buffer->layers, layer);
                }
            } else if (this_layer->z_index < z_index && !current->next) {
                layer = DrawBufferLayer_create(z_index);
                List_push(buffer->layers, layer);
            }
        }
    } else {
        layer = DrawBufferLayer_create(z_index);
        List_push(buffer->layers, layer);
    }
    
    DrawBufferLayer_buffer(layer, texture, num_points, num_attrs, vectors);
}

void DrawBuffer_empty(DrawBuffer *buffer) {
    assert(buffer != NULL);
    if (!buffer->layers) return;
    
    LIST_FOREACH(buffer->layers, first, next, current) {
        DrawBufferLayer_destroy(current->value);
    }
    List_destroy(buffer->layers);
    buffer->layers = NULL;
}

void DrawBuffer_draw(DrawBuffer *buffer) {
    assert(buffer != NULL);
    if (!buffer->layers) return;
    
    LIST_FOREACH(buffer->layers, first, next, current) {
        DrawBufferLayer_draw(current->value);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

