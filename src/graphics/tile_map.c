#include <stdio.h>
#include <lcthw/dbg.h>
#include "tile_map.h"
#include "../physics/world.h"
#include "../physics/world_grid.h"

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

void TileMapLayer_destroy(TileMapLayer *layer) {
  check(layer != NULL, "No layer to destroy");
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
  int ts_cols, ts_rows;
  for (i = 0; i < map->tilesets->end; i++) {
    Tileset *tileset = DArray_get(map->tilesets, i);
    ts_cols = (tileset->texture->size.w - 2 * tileset->margin) /
                  (tileset->tile_size.w + tileset->spacing);
    ts_rows = (tileset->texture->size.h - 2 * tileset->margin) /
                  (tileset->tile_size.h + tileset->spacing);
    
    int min_gid = tileset->first_gid;
    int max_gid = tileset->first_gid + ts_cols * ts_rows - 1;
    if (gid >= min_gid && gid <= max_gid) {
      matched = tileset;
      break;
    }
  }
  check(matched != NULL, "No matching tileset found for gid %u", gid);
  
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
    GLfloat tileColor[4] = {0, 0, 0, 0};
    Graphics_reset_modelview_matrix(graphics);
    int layer_idx = 0;
    double scale = pixels_per_cell / map->tile_size.w;
    for (layer_idx = 0; layer_idx < map->layers->end;
         layer_idx++) {
      TileMapLayer *layer = DArray_get(map->layers, layer_idx);
      int gid_idx = 0;
      for (gid_idx = 0; gid_idx < layer->gid_count; gid_idx++) {
        uint32_t gid = layer->tile_gids[gid_idx];
        if (gid != 0) {
          TilesetTile *tile = TileMap_resolve_tile_gid(map, gid);
          if (tile == NULL) continue;
          
          int gid_col = gid_idx % (int)map->cols;
          int gid_row = gid_idx / (int)map->cols;
          GfxRect tile_rect =
              GfxRect_from_xywh(gid_col * tile->size.w * scale,
                                gid_row * tile->size.h * scale,
                                tile->size.w * scale,
                                tile->size.h * scale);
          
          Graphics_draw_rect(graphics, tile_rect, tileColor,
                             tile->tileset->texture, tile->tl,
                             tile->size, 0);
          free(tile);
        }
      }
    }
}
