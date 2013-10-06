#include "draw_buffer.h"

DrawBuffer *DrawBuffer_create() {
    DrawBuffer *buffer = calloc(1, sizeof(DrawBuffer));
    check(buffer != NULL, "Couldn't create Draw Buffer");
  
    buffer->texture_buffers = Hashmap_create(NULL, Hashmap_fnv1a_hash);
    buffer->textures = DArray_create(sizeof(DrawBufferTexture *), 10);
  
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

void DrawBuffer_buffer(DrawBuffer *buffer, GfxTexture *texture,
                       int num_points, int num_attrs, VVector4 vectors[]) {
    assert(buffer != NULL);
  
    char key[32];
    int result = sprintf(key, "%p", texture);
    assert(result < 31);
    
    bstring bkey = bfromcstr(key);
    
    int i = 0;
    int found = 0;
    DArray *shapes = NULL;
    if (buffer->textures) {
        for (i = 0; i < DArray_count(buffer->textures); i++) {
            DrawBufferTexture *buftex = DArray_get(buffer->textures, i);
            if (bstrcmp(buftex->key, bkey) != 0) continue;
                
            found = 1;
            shapes = Hashmap_get(buffer->texture_buffers, bkey);
            bdestroy(bkey);
            break;
        }
    }
  
    if (found == 0) {
        DrawBufferTexture *buftex = calloc(1, sizeof(DrawBufferTexture));
        check(buftex != NULL, "Couldn't create draw buffer texture");
        buftex->texture = texture;
        buftex->key = bkey;
        DArray_push(buffer->textures, buftex);
        
        shapes = DArray_create(sizeof(DrawBufferShape *), 24);
        Hashmap_set(buffer->texture_buffers, bkey, shapes);
    }
    
    int num_vectors = num_attrs * num_points;
    DrawBufferShape *shape = calloc(1, sizeof(DrawBufferShape) +
                                    sizeof(VVector4) * num_vectors);
    check(shape != NULL, "Couldn't create draw buffer shape");
    
    shape->num_points = num_points;
    shape->num_vectors = num_vectors;
    memcpy(shape->vectors, vectors, sizeof(VVector4) * num_vectors);
    
    DArray_push(shapes, shape);
  
    buffer->populated = 1;
  
    return;
error:
    return;
}

void DrawBuffer_empty(DrawBuffer *buffer) {
    assert(buffer != NULL);
  
    if (buffer->populated) {
      buffer->populated = 0;
      DArray_clear_destroy(buffer->textures);
      buffer->textures = DArray_create(sizeof(DrawBufferTexture *), 10);
      Hashmap_destroy(buffer->texture_buffers,
                      (Hashmap_destroy_func)DArray_clear_destroy);
      buffer->texture_buffers = Hashmap_create(NULL, Hashmap_fnv1a_hash);
    }
}

void DrawBuffer_draw(DrawBuffer *buffer, Graphics *graphics) {
    assert(buffer != NULL);
  
    if (!buffer->populated) {
      return;
    }
  
    int i = 0;
    
    for (i = 0; i < DArray_count(buffer->textures); i++) {
        DrawBufferTexture *buftex = DArray_get(buffer->textures, i);
        DArray *texshapes = Hashmap_get(buffer->texture_buffers, buftex->key);
        int j = 0;
        int num_points = 0;
        int num_vectors = 0;
        for (j = 0; j < DArray_count(texshapes); j++) {
            DrawBufferShape *shape = DArray_get(texshapes, j);
            num_points += shape->num_points;
            num_vectors += shape->num_vectors;
        }
        if (num_vectors == 0) continue;
      
        // Copy the shapes into an inline buffer
        VVector4 vectors[num_vectors];
        VVector4 *v_head = vectors;
        for (j = 0; j < DArray_count(texshapes); j++) {
            DrawBufferShape *shape = DArray_get(texshapes, j);
            memcpy(v_head, shape->vectors,
                   sizeof(VVector4) * shape->num_vectors);
            v_head += shape->num_vectors;
        }
        
        GLint gl_tex = 0;
      
        // I don't think this shader needs this set.
        // glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_TEXTURE], 0);
      
        if (buftex->texture) {
            gl_tex = buftex->texture->gl_tex;
            Graphics_uniform1f(graphics,
                               UNIFORM_DECAL_TEX_ALPHA_ADJ,
                               1.0);
            glBindTexture(GL_TEXTURE_2D, gl_tex);
        } else {
            Graphics_uniform1f(graphics,
                               UNIFORM_DECAL_TEX_ALPHA_ADJ,
                               0.0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glBufferData(GL_ARRAY_BUFFER, num_vectors * sizeof(VVector4), vectors,
                GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, num_points);
    }
}

