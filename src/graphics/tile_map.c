#include <stdio.h>
#include <lcthw/dbg.h>
#include "tile_map.h"
#include "../physics/world.h"
#include "../physics/world_grid.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static const uint32_t rmask = 0xff000000;
    static const uint32_t gmask = 0x00ff0000;
    static const uint32_t bmask = 0x0000ff00;
    static const uint32_t amask = 0x000000ff;
#else
    static const uint32_t rmask = 0x000000ff;
    static const uint32_t gmask = 0x0000ff00;
    static const uint32_t bmask = 0x00ff0000;
    static const uint32_t amask = 0xff000000;
#endif

const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
const unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
const unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;

TileMapLayer *TileMapLayer_create() {
  TileMapLayer *layer = calloc(1, sizeof(TileMapLayer));
  check(layer != NULL, "Couldn't create layer");

  layer->opacity = 1;
  layer->visible = 1;

  return layer;
error:
  if (layer) TileMapLayer_destroy(layer);
  return NULL;
}

GfxTexture *TileMapLayer_create_atlas(TileMapLayer *layer, TileMap *map) {
  int i = 0;
  layer->raw_atlas = malloc(sizeof(uint8_t) * layer->gid_count * 4);
  for (i = 0; i < layer->gid_count * 4; i += 4) {
    int tile_idx = i / 4;
    TilesetTile *tile = TileMap_resolve_tile_gid(map, layer->tile_gids[tile_idx]);
    uint32_t first_gid = 0;
    uint32_t diff_gid = UINT32_MAX;
    if (tile) {
      layer->tileset = tile->tileset;
      first_gid = tile->tileset->first_gid;
      diff_gid = layer->tile_gids[tile_idx] - first_gid;
    }

    layer->raw_atlas[i]   = (diff_gid & 0x000000ff);
    layer->raw_atlas[i+1] = (diff_gid & 0x0000ff00) >> 8;
    layer->raw_atlas[i+2] = (diff_gid & 0x00ff0000) >> 16;
    layer->raw_atlas[i+3] = (diff_gid & 0xff000000) >> 24;
  }
  return GfxTexture_from_data((unsigned char **)&layer->raw_atlas,
                              map->cols, map->rows, GL_RGBA);
}

void TileMapLayer_dump_raw_atlas(TileMapLayer *layer) {
  if (layer == NULL) return;
  if (layer->raw_atlas == NULL) return;
  int i = 0;
  for (i = 0; i < layer->gid_count * 4; i += 4) {
    printf("%d %d %d %d\n", layer->raw_atlas[i], layer->raw_atlas[i+1],
           layer->raw_atlas[i+2], layer->raw_atlas[i+3]);
  }
}

void TileMapLayer_destroy(TileMapLayer *layer) {
  check(layer != NULL, "No layer to destroy");
  if (layer->atlas) GfxTexture_destroy(layer->atlas);
  if (layer->tile_gids != NULL) free(layer->tile_gids);
  free(layer);
  return;
error:
  return;
}

void Tileset_destroy(Tileset *tileset) {
  if (tileset->name != NULL) free(tileset->name);
  free(tileset);
}

TileMap *TileMap_create() {
  TileMap *map = calloc(1, sizeof(TileMap));
  check(map != NULL, "Couldn't create map");

  map->tilesets = DArray_create(sizeof(Tileset), 8);
  map->tilesets->expand_rate = 8;

  map->layers = DArray_create(sizeof(TileMapLayer), 8);
  map->layers->expand_rate = 8;

  return map;

error:
  if (map) free(map);
  return NULL;
}

void TileMap_destroy(TileMap *map) {
  int i = 0;
  for (i = 0; i < map->tilesets->end; i++) {
    Tileset_destroy(DArray_get(map->tilesets, i));
  }
  DArray_destroy(map->tilesets);

  for (i = 0; i < map->layers->end; i++) {
    TileMapLayer_destroy(DArray_get(map->layers, i));
  }
  DArray_destroy(map->layers);
  free(map);
}

TilesetTile *TileMap_resolve_tile_gid(TileMap *map, uint32_t gid) {
  int i = 0;
  Tileset *matched = NULL;
  unsigned int ts_cols, ts_rows;
  for (i = 0; i < map->tilesets->end; i++) {
    Tileset *tileset = DArray_get(map->tilesets, i);
    if (tileset->texture == NULL) continue;
    ts_cols = (tileset->texture->size.w - 2 * tileset->margin) /
                  (tileset->tile_size.w + tileset->spacing);
    ts_rows = (tileset->texture->size.h - 2 * tileset->margin) /
                  (tileset->tile_size.h + tileset->spacing);

    unsigned int min_gid = tileset->first_gid;
    unsigned int max_gid = tileset->first_gid + ts_cols * ts_rows - 1;
    if (gid >= min_gid && gid <= max_gid) {
      matched = tileset;
      break;
    }
  }
  if (matched == NULL) return NULL;

  int gid_idx = gid - matched->first_gid;
  int gid_col = gid_idx % ts_cols;
  int gid_row = gid_idx / ts_cols;

  TilesetTile *tile = malloc(sizeof(TilesetTile));
  check(tile != NULL, "Couldn't create tile");
  tile->tileset = matched;
  tile->gid = gid;
  tile->tl.x = (gid_col * (matched->tile_size.w + matched->spacing)
                   + matched->margin);
  tile->tl.y = (gid_row * (matched->tile_size.h + matched->spacing)
                   + matched->margin);
  tile->size = matched->tile_size;

  return tile;
error:
  return NULL;
}

void TileMap_render(TileMap *map, Graphics *graphics, int pixels_per_cell) {
    if (map == NULL) return;

    Graphics_reset_modelview_matrix(graphics);
    int layer_idx = 0;
    for (layer_idx = 0; layer_idx < map->layers->end;
         layer_idx++) {
      TileMapLayer *layer = DArray_get(map->layers, layer_idx);
      if (!layer->atlas) {
        layer->atlas = TileMapLayer_create_atlas(layer, map);
      }

      VRect rect = VRect_from_xywh(0, 0,
                                   map->cols * pixels_per_cell,
                                   map->rows * pixels_per_cell);
      Graphics_reset_modelview_matrix(graphics);
      double w = rect.tr.x - rect.tl.x;
      double h = rect.bl.y - rect.tl.y;
      VPoint center = {
          rect.tl.x + w / 2,
          rect.tl.y + h / 2
      };
      Graphics_translate_modelview_matrix(graphics, center.x, center.y, 0.f);

      GfxUVertex tex_tl = {.raw = {0,0,0,0}};
      GfxUVertex tex_tr = {.raw = {1,0,0,0}};
      GfxUVertex tex_bl = {.raw = {0,1,0,0}};
      GfxUVertex tex_br = {.raw = {1,1,0,0}};
      glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_TILEMAP_PROJECTION_MATRIX],
                         1, GL_FALSE, graphics->projection_matrix.gl);
      glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_TILEMAP_MODELVIEW_MATRIX],
                         1, GL_FALSE, graphics->modelview_matrix.gl);

      if (layer->atlas) {
          glUniform2f(GfxShader_uniforms[UNIFORM_TILEMAP_MAP_ROWS_COLS],
                      layer->atlas->pot_size.w, layer->atlas->pot_size.h);

          glUniform1i(GfxShader_uniforms[UNIFORM_TILEMAP_ATLAS], 0);
          glActiveTexture(GL_TEXTURE0);
          glBindTexture(GL_TEXTURE_2D, layer->atlas->gl_tex);

          VPoint pot_scale = {
              layer->atlas->size.w / layer->atlas->pot_size.w,
              layer->atlas->size.h / layer->atlas->pot_size.h
          };
          tex_tl.packed.x *= pot_scale.x;
          tex_tr.packed.x *= pot_scale.x;
          tex_bl.packed.x *= pot_scale.x;
          tex_br.packed.x *= pot_scale.x;
          tex_tl.packed.y *= pot_scale.y;
          tex_tr.packed.y *= pot_scale.y;
          tex_bl.packed.y *= pot_scale.y;
          tex_br.packed.y *= pot_scale.y;
      }

      if (layer->tileset) {
          glUniform2f(GfxShader_uniforms[UNIFORM_TILEMAP_TILE_SIZE],
                      layer->tileset->tile_size.w, layer->tileset->tile_size.h);
          glUniform2f(GfxShader_uniforms[UNIFORM_TILEMAP_SHEET_ROWS_COLS],
              layer->tileset->texture->size.w / layer->tileset->tile_size.w,
              layer->tileset->texture->size.h / layer->tileset->tile_size.h
          );

          glUniform2f(GfxShader_uniforms[UNIFORM_TILEMAP_SHEET_POT_SIZE],
              layer->tileset->texture->pot_size.w / layer->tileset->tile_size.w,
              layer->tileset->texture->pot_size.h / layer->tileset->tile_size.h
          );

          if (layer->tileset->texture) {
              glUniform1i(GfxShader_uniforms[UNIFORM_TILEMAP_TILESET], 1);
              glActiveTexture(GL_TEXTURE1);
              glBindTexture(GL_TEXTURE_2D, layer->tileset->texture->gl_tex);
          }
      }

      GfxUVertex vertices[12] = {
        // Vertex
        {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
        {.raw = {w / 2.0, -h / 2.0, 0, 1}},
        {.raw = {-w / 2.0, h / 2.0, 0, 1}},
        {.raw = {w / 2.0, h / 2.0, 0, 1}},

        // Texture
        tex_tl, tex_tr, tex_bl, tex_br
      };
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, 0);
    }
}
