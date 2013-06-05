#ifndef __ortho_chipmunk_scene_h
#define __ortho_chipmunk_scene_h
#include "scene.h"

typedef enum {
  OCSCollisionTypeEntity = 1,
  OCSCollisionTypeTile = 2,
  OCSCollisionTypeDrawShape = 3
} OCSCollisionType;

extern SceneProto OrthoChipmunkSceneProto;

#endif
