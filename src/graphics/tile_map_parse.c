#include <lcthw/bstrlib.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlreader.h>
#include "tile_map_parse.h"
#include "../core/base64.h"
#include "../core/gzip.h"
#include "../core/engine.h"

#define streq(A, B) (strcmp((const char *)A, (const char *)B) == 0)

void extract_gids_from_encoded_data(xmlChar *value, uint32_t **gids,
                                     int *gid_count) {
    unsigned char *decoded = malloc(1024);
    size_t outlen = base64_decode((char *)value,
                                  &decoded,
                                  1024);
    unsigned char *decompressed = calloc(1, 256);
    int unzip_len = decompress_data(decoded, outlen, &decompressed, 256);
    free(decoded);
    int i = 0;
    *gids = malloc(sizeof(uint32_t) * (unzip_len / 4));
    int cell = 0;
    for (i = 0; i < unzip_len; i += 4) {
      uint32_t gid = (decompressed[i] | decompressed[i + 1] << 8 |
                      decompressed[i + 2] << 16 | decompressed[i + 3] << 24);
      (*gids)[cell] = gid;
      cell += 1;
    }
    free(decompressed);
    *gid_count = cell;
}

TileMapParseStatus TileMapLayer_parse_data(xmlTextReaderPtr reader,
                                           TileMap *map, TileMapLayer *layer) {
  TileMapParseStatus status = TILEMAP_PARSE_OK;
  
  while (xmlTextReaderMoveToNextAttribute(reader)) {
    xmlChar *attrName = xmlTextReaderName(reader);
    xmlChar *attrVal = xmlTextReaderValue(reader);
    
    if (streq(attrName, "encoding")) {
      check(streq(attrVal, "base64"), "Incorrect layer data encoding");
    } else if (streq(attrName, "compression")) {
      check(streq(attrVal, "gzip"), "Incorrect layer data compression");
    }
    xmlFree(attrName);
    xmlFree(attrVal);
  }
  
  while (xmlTextReaderRead(reader)) {
    xmlChar *childName = xmlTextReaderName(reader);
    if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
        streq(childName, "data")) {
      xmlFree(childName);
      break;
    } else if (xmlTextReaderNodeType(reader) == XML_TEXT_NODE) {
      int tile_count = 0;
      xmlChar *data = xmlTextReaderValue(reader);
      extract_gids_from_encoded_data(data,
                                     &(layer->tile_gids), &tile_count);
      xmlFree(data);
      check(tile_count == map->rows * map->cols, "Inconsistent layer size");
      layer->gid_count = tile_count;
    }
    xmlFree(childName);
  }
  
  return status;
error:
  return TILEMAP_PARSE_INVALID_FORMAT;
}

TileMapParseStatus TileMap_parse_layer(xmlTextReaderPtr reader, TileMap *map,
                                       TileMapLayer **out_layer) {
  TileMapParseStatus status = TILEMAP_PARSE_OK;
  TileMapLayer *layer = TileMapLayer_create();
  check(layer != NULL, "Couldn't create layer");
  
  while (xmlTextReaderMoveToNextAttribute(reader)) {
    xmlChar *attrName = xmlTextReaderName(reader);
    xmlChar *attrVal = xmlTextReaderValue(reader);
    
    if (streq(attrName, "name")) {
      layer->name = calloc(1, strlen((const char *)attrVal) + 1);
      strcpy(layer->name, (const char *)attrVal);
    } else if (streq(attrName, "opacity")) {
      layer->opacity = atof((const char *)attrVal);
    } else if (streq(attrName, "visible")) {
      layer->visible = atoi((const char *)attrVal);
    }
    
    xmlFree(attrName);
    xmlFree(attrVal);
  }
  
  while (xmlTextReaderRead(reader)) {
    xmlChar *childName = xmlTextReaderName(reader);
    if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
        streq(childName, "layer")) {
      xmlFree(childName);
      break;
    } else if (streq(childName, "data")) {
      status = TileMapLayer_parse_data(reader, map, layer);
      check(status == TILEMAP_PARSE_OK, "Failed to parse layer data");
    }
    xmlFree(childName);
  }
  
  if (status == TILEMAP_PARSE_OK) {
    *out_layer = layer;
    return status;
  }
error:
  if (layer) TileMapLayer_destroy(layer);
  if (status == TILEMAP_PARSE_OK) status = TILEMAP_PARSE_UNKNOWN_ERR;
  return status;
}

TileMapParseStatus TileMap_parse_tileset(xmlTextReaderPtr reader,
                                         Engine *engine, TileMap *map,
                                         Tileset **out_tileset) {
  TileMapParseStatus status = TILEMAP_PARSE_OK;
  Tileset *tileset = calloc(1, sizeof(Tileset));
  check(tileset != NULL, "Couldn't create tileset");
  
  while (xmlTextReaderMoveToNextAttribute(reader)) {
    xmlChar *attrName = xmlTextReaderName(reader);
    xmlChar *attrVal = xmlTextReaderValue(reader);
    
    if (streq(attrName, "firstgid")) {
      tileset->first_gid = atoi((const char *)attrVal);
    } else if (streq(attrName, "tilewidth")) {
      tileset->tile_size.w = atoi((const char *)attrVal);
    } else if (streq(attrName, "tileheight")) {
      tileset->tile_size.h = atoi((const char *)attrVal);
    } else if (streq(attrName, "name")) {
      tileset->name = calloc(1, strlen((const char *)attrVal) + 1);
      strcpy(tileset->name, (const char *)attrVal);
    }
    
    xmlFree(attrName);
    xmlFree(attrVal);
  }
  while (xmlTextReaderRead(reader)) {
    xmlChar *childName = xmlTextReaderName(reader);
    if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
        streq(childName, "tileset")) {
      xmlFree(childName);
      break;
    } else if (streq(childName, "image")) {
      while (xmlTextReaderMoveToNextAttribute(reader)) {
        xmlChar *attrName = xmlTextReaderName(reader);
        xmlChar *attrVal = xmlTextReaderValue(reader);
        
        if (streq(attrName, "source")) {
          // Check for existence of image
          bstring imgpath = bfromcstr("media/tilesets/");
          bstring src = bfromcstr((const char *)attrVal);
          bconcat(imgpath, src);
          char *cpath = bstr2cstr(imgpath, '\0');
          bdestroy(imgpath);
          bdestroy(src);
          
          FILE *fileexists = load_resource(cpath);
          if (fileexists == NULL) {
            free(cpath);
            Tileset_destroy(tileset);
            Engine_log("Cannot open map. Missing tileset <%s>", attrVal);
            xmlFree(attrName);
            xmlFree(attrVal);
            xmlFree(childName);
            return TILEMAP_PARSE_MISSING_IMAGE;
          }
          fclose(fileexists);
          
          tileset->texture = Graphics_texture_from_image(engine->graphics,
                                                         cpath);
          tileset->img_src = cpath;
        }
        
        xmlFree(attrName);
        xmlFree(attrVal);
      }
      xmlFree(childName);
    }
  }
  
  *out_tileset = tileset;
  
  return status;
error:
  if (tileset) Tileset_destroy(tileset);
  if (status == TILEMAP_PARSE_OK) status = TILEMAP_PARSE_UNKNOWN_ERR;
  return status;
}

TileMapParseStatus TileMap_parse_map(xmlTextReaderPtr reader, Engine *engine,
                                     TileMap *map) {
  TileMapParseStatus status = TILEMAP_PARSE_OK;
  
  xmlChar *name = xmlTextReaderName(reader);
  if (!(streq(name, "map") &&
        xmlTextReaderNodeType(reader) == XML_ELEMENT_NODE)) {
    xmlFree(name);
    return TILEMAP_PARSE_INVALID_FORMAT;
  }
  xmlFree(name);
  
  while (xmlTextReaderMoveToNextAttribute(reader)) {
    xmlChar *attrName = xmlTextReaderName(reader);
    xmlChar *attrVal = xmlTextReaderValue(reader);
    
    if (streq(attrName, "orientation")) {
      if (!streq(attrVal, "orthogonal")) {
        xmlFree(attrName);
        xmlFree(attrVal);
        return TILEMAP_PARSE_INVALID_ORIENTATION;
      }
    } else if (streq(attrName, "width")) {
      map->cols = atoi((const char *)attrVal);
    } else if (streq(attrName, "height")) {
      map->rows = atoi((const char *)attrVal);
    } else if (streq(attrName, "tilewidth")) {
      map->tile_size.w = atoi((const char *)attrVal);
    } else if (streq(attrName, "tileheight")) {
      map->tile_size.h = atoi((const char *)attrVal);
    }
    xmlFree(attrName);
    xmlFree(attrVal);
  }
  
  while (xmlTextReaderRead(reader)) {
    xmlChar *childName = xmlTextReaderName(reader);
    if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
        streq(childName, "map")) {
      xmlFree(childName);
      break;
    } else if (streq(childName, "tileset")) {
      Tileset *tileset = NULL;
      status = TileMap_parse_tileset(reader, engine, map, &tileset);
      if (status != TILEMAP_PARSE_OK) return status;
      DArray_push(map->tilesets, tileset);
    } else if (streq(childName, "layer")) {
      TileMapLayer *layer = NULL;
      status = TileMap_parse_layer(reader, map, &layer);
      if (status != TILEMAP_PARSE_OK) {
          xmlFree(childName);
          return status;
      }
      DArray_push(map->layers, layer);
    }
    xmlFree(childName);
  }
  
  return status;
}

TileMap *TileMap_parse(char *filename, Engine *engine) {
  TileMap *map = TileMap_create();

  xmlTextReaderPtr reader;
  int ret;
  TileMapParseStatus status = TILEMAP_PARSE_OK;

  reader = xmlNewTextReaderFilename(filename);
  if (reader != NULL) {
    ret = xmlTextReaderRead(reader);
    while (ret == 1) {
      status = TileMap_parse_map(reader, engine, map);
      ret = xmlTextReaderRead(reader);
    }
    xmlFreeTextReader(reader);
    if (ret != 0) {
      printf("%s : failed to parse\n", filename);
    }
  } else {
    printf("Unable to open %s", filename);
  }
  
  check(status == TILEMAP_PARSE_OK, "Error parsing tile map");
  return map;
error:
  TileMap_destroy(map);
  return NULL;
}
