#include <lcthw/bstrlib.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlreader.h>
#include "tile_map_parse.h"
#include "../core/base64.h"
#include "../core/gzip.h"
#include "../core/engine.h"
#include "../math/vpolygon.h"

void str_sanitize(char **str) {
    char *string = *str;
    bstring bstr = bfromcstr((const char *)string);
    bstring space = bfromcstr(" ");
    bstring nl = bfromcstr("\n");
    bstring empty = bfromcstr("");
    int replaced = 0;
    replaced = bfindreplace(bstr, nl, empty, 0);
    replaced = bfindreplace(bstr, space, empty, 0);
    char *processed = bstr2cstr(bstr, '\0');
    *str = realloc(string, strlen(processed) + 1);
    strcpy(*str, processed);
    bdestroy(bstr);
    bdestroy(space);
    bdestroy(nl);
    bdestroy(empty);
}

void extract_gids_from_encoded_data(xmlChar *value, uint32_t **gids,
        int *gid_count) {
    unsigned char *decoded = NULL;
    int outlen = 0;
    int inlen = xmlStrlen(value);
  
    char *to_decode = calloc(1, inlen + 1);
    strcpy(to_decode, (const char *)value);
    str_sanitize(&to_decode);
    inlen = (int)strlen(to_decode);
    decoded = base64_decode((const void *)to_decode, inlen, &outlen);
  
    unsigned char *decompressed = calloc(1, 256);
    unsigned long int unzip_len = decompress_data(decoded, outlen, &decompressed, 256);
    free(decoded);
    unsigned int i = 0;
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
        Engine *engine, TileMap *UNUSED(map),
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

                    char *ppath = engine->project_path(cpath);
                    FILE *fileexists = fopen(ppath, "r");
                    free(ppath);
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

                    ppath = engine->project_path(cpath);
                    tileset->texture = Graphics_texture_from_image(engine->graphics, ppath);
                    free(ppath);
                    tileset->img_src = cpath;
                }

                xmlFree(attrName);
                xmlFree(attrVal);
            }
        }
        xmlFree(childName);
    }

    *out_tileset = tileset;

    return status;
error:
    if (tileset) Tileset_destroy(tileset);
    if (status == TILEMAP_PARSE_OK) status = TILEMAP_PARSE_UNKNOWN_ERR;
    return status;
}

void parse_poly(xmlChar *attrVal, VPolygon **poly, TileMap *map) {
    size_t len = strlen((char *)attrVal);
    char val[len + 1];
    strcpy(val, (char *)attrVal);
    val[len] = '\0';

    int num_points = 0;
    VPoint *points = NULL;
    char *token = strtok(val, " ");
    while (token != NULL) {
        VPoint point = VPointZero;
        points = realloc(points, sizeof(VPoint) * (num_points + 1));
        sscanf(token, "%f,%f", &point.x, &point.y);
        point.x /= map->tile_size.w;
        point.y /= map->tile_size.h;
        points[num_points] = point;
        num_points++;
        token = strtok(NULL, " ");
    }

    VPolygon *npoly = VPolygon_create(num_points, points);
    free(points);
    *poly = npoly;
}

TileMapParseStatus TileMap_parse_objectgroup(xmlTextReaderPtr reader, TileMap *map) {
    TileMapParseStatus status = TILEMAP_PARSE_OK;

    int is_collision = 0;
    int is_spawn = 0;
    while (xmlTextReaderRead(reader)) {
        xmlChar *childName = xmlTextReaderName(reader);
        if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
                streq(childName, "objectgroup")) {
            xmlFree(childName);
            break;
        } else if (streq(childName, "properties")) {
            while (xmlTextReaderRead(reader)) {
                xmlChar *childName = xmlTextReaderName(reader);
                if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
                        streq(childName, "properties")) {
                    xmlFree(childName);
                    break;
                } else if (streq(childName, "property")) {
                    while (xmlTextReaderMoveToNextAttribute(reader)) {
                        xmlChar *attrName = xmlTextReaderName(reader);
                        xmlChar *attrVal = xmlTextReaderValue(reader);

                        if (streq(attrName, "name")) {
                            if (streq(attrVal, "spawn")) {
                                is_spawn = 1;
                            } else if (streq(attrVal, "collision")) {
                                is_collision = 1;
                            }
                        }

                        xmlFree(attrName);
                        xmlFree(attrVal);
                    }
                }
                xmlFree(childName);
            }
        } else if (streq(childName, "object")) {
            VPolygon *poly = NULL;
            float x, y;
            float w = -1;
            float h = -1;
            while (xmlTextReaderMoveToNextAttribute(reader)) {
                xmlChar *attrName = xmlTextReaderName(reader);
                xmlChar *attrVal = xmlTextReaderValue(reader);

                if (streq(attrName, "x")) {
                    x = atof((char *)attrVal) / map->tile_size.w;
                } else if (streq(attrName, "y")) {
                    y = atof((char *)attrVal) / map->tile_size.h;
                } else if (streq(attrName, "width")) {
                    w = atof((char *)attrVal) / map->tile_size.w;
                } else if (streq(attrName, "height")) {
                    h = atof((char *)attrVal) / map->tile_size.h;
                }

                xmlFree(attrName);
                xmlFree(attrVal);
            }

            VPoint origin = {x, y};
            if (w >= 0 || h >= 0) {
                // Rect
                VRect rect = VRect_from_xywh(0, 0, w, h);
                poly = VPolygon_from_rect(rect);
                poly->origin = origin;
            } else {
                // Poly
                while (xmlTextReaderRead(reader)) {
                    xmlChar *childName = xmlTextReaderName(reader);
                    if (xmlTextReaderNodeType(reader) == XML_ELEMENT_DECL &&
                            streq(childName, "object")) {
                        xmlFree(childName);
                        break;
                    } else if (streq(childName, "polygon")) {
                        while (xmlTextReaderMoveToNextAttribute(reader)) {
                            xmlChar *attrName = xmlTextReaderName(reader);
                            xmlChar *attrVal = xmlTextReaderValue(reader);

                            if (streq(attrName, "points")) {
                                parse_poly(attrVal, &poly, map);
                                poly->origin = origin;
                            }

                            xmlFree(attrName);
                            xmlFree(attrVal);
                        }
                    }
                    xmlFree(childName);
                }
            }

            if (is_collision) {
                check(poly != NULL, "No poly to push");
                if (map->collision_shapes == NULL) {
                    map->collision_shapes = DArray_create(sizeof(VPolygon), 64);
                }

                DArray_push(map->collision_shapes, poly);
            }
        }
        xmlFree(childName);
    }
    return status;
error:
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
        } else if (streq(childName, "objectgroup")) {
            status = TileMap_parse_objectgroup(reader, map);
            if (status != TILEMAP_PARSE_OK) {
                xmlFree(childName);
                return status;
            }
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
