#ifndef __tile_map_parse_h
#define __tile_map_parse_h
#include "tile_map.h"
#include "../core/engine.h"

typedef enum {
  TILEMAP_PARSE_OK = 0,
  TILEMAP_PARSE_INVALID_FORMAT = 1,
  TILEMAP_PARSE_INVALID_ORIENTATION = 2,
  TILEMAP_PARSE_MISSING_IMAGE = 3,
  TILEMAP_PARSE_UNKNOWN_ERR = 4
} TileMapParseStatus;

TileMap *TileMap_parse(char *filename, Engine *engine);

#endif
