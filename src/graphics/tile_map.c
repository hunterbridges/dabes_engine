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

void TileMapLayer_destroy(TileMapLayer *layer) {
  check(layer != NULL, "No layer to destroy");
  if (layer->tile_gids != NULL) free(layer->tile_gids);
  free(layer);
  return;
error:
  return;
}

void TileMapLayer_draw(TileMapLayer *layer, TileMap *map, Graphics *graphics) {
    int gid_idx = 0;
    int width = map->cols * map->tile_size.w;
    int height = map->rows * map->tile_size.h;
#ifdef DABES_IOS
    CGImageRef tileset_img = NULL;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    unsigned char *rawData = calloc(1, height * width * 4);
    int bytesPerPixel = 4;
    int bytesPerRow = bytesPerPixel * width;
    int bitsPerComponent = 8;
    CGContextRef context =
        CGBitmapContextCreate(rawData, width, height, bitsPerComponent,
                              bytesPerRow, colorSpace,
                              kCGImageAlphaPremultipliedLast |
                              kCGBitmapByteOrder32Big);
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1.0, -1.0);
#else
    SDL_Surface *tileset_img = NULL;
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width,
        height, 32, rmask, gmask, bmask, amask);
#endif
  
    for (gid_idx = 0; gid_idx < layer->gid_count; gid_idx++) {
      uint32_t gid = layer->tile_gids[gid_idx];
      if (gid != 0) {
        TilesetTile *tile = TileMap_resolve_tile_gid(map, gid);
        if (tile == NULL) continue;
        int gid_col = gid_idx % (int)map->cols;
        int gid_row = gid_idx / (int)map->cols;
#ifdef DABES_IOS
        if (tileset_img == NULL) {
          tileset_img = Graphics_load_CGImage(tile->tileset->img_src);
        }
        
        CGContextSaveGState(context);
        CGRect clipRect = CGRectMake(gid_col * tile->size.w,
                                     gid_row * tile->size.h,
                                     tile->size.w,
                                     tile->size.h);
        
        CGRect imgRect = CGRectMake(clipRect.origin.x - tile->tl.x,
                                    clipRect.origin.y - tile->tl.y,
                                    CGImageGetWidth(tileset_img),
                                    CGImageGetHeight(tileset_img));
        
        // WOW annoying
        CGContextClipToRect(context, clipRect);
        CGContextTranslateCTM(context, 0, tile->tileset->texture->size.h);
        CGContextScaleCTM(context, 1.0, -1.0);
        imgRect.origin.y *= -1;
        CGContextDrawImage(context, imgRect, tileset_img);
        CGContextRestoreGState(context);
#else
        if (tileset_img == NULL) {
          tileset_img = Graphics_load_SDLImage(tile->tileset->img_src);
          SDL_SetAlpha(tileset_img, 0, 255);
          SDL_SetColorKey(tileset_img, 0, 0);
        }
        
        SDL_Rect srcRect = {tile->tl.x, tile->tl.y, tile->size.w, tile->size.h};
        SDL_Rect dstRect = {gid_col * tile->size.w, gid_row * tile->size.h,
            tile->size.w, tile->size.h};
        SDL_BlitSurface(tileset_img, &srcRect, surface, &dstRect);
#endif
        free(tile);
      }
    }
  
#ifdef DABES_IOS
    CGContextRelease(context);
    CGDataProviderRef dataProvider =
        CGDataProviderCreateWithData(NULL, rawData, bytesPerRow * height, NULL);
    CGImageRef cgImage = CGImageCreate(width, height, bitsPerComponent,
                                       bitsPerComponent * 4, bytesPerRow,
                                       colorSpace,
                                       kCGImageAlphaPremultipliedLast |
                                           kCGBitmapByteOrder32Big,
                                       dataProvider, NULL, false,
                                       kCGRenderingIntentDefault);
    layer->texture = GfxTexture_from_CGImage(cgImage);
    if (tileset_img) CGImageRelease(tileset_img);
    CGDataProviderRelease(dataProvider);
    CGColorSpaceRelease(colorSpace);
    free(rawData);
#else
    layer->texture = GfxTexture_from_surface(surface);
    if (tileset_img) SDL_FreeSurface(tileset_img);
#endif
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
    Graphics_reset_modelview_matrix(graphics);
    int layer_idx = 0;
    for (layer_idx = 0; layer_idx < map->layers->end;
         layer_idx++) {
      TileMapLayer *layer = DArray_get(map->layers, layer_idx);
      if (layer->texture == NULL) {
          TileMapLayer_draw(layer, map, graphics);
      }
      VRect layer_rect = VRect_from_xywh(0, 0,
                                         map->cols * pixels_per_cell,
                                         map->rows * pixels_per_cell);
      GLfloat color[4] = {0,0,1,0};
      Graphics_draw_rect(graphics, layer_rect, color, layer->texture,
                         VPointZero, GfxSizeZero, 0);
    }
}
