#ifndef __chipmunk_scene_h
#define __chipmunk_scene_h
#include "scene.h"

#define COLLISION_SLOP 0.1

typedef struct {
    Scene *scene;
    Engine *engine;
} OCSIterData;

typedef enum {
  OCSCollisionTypeEntity = 1,
  OCSCollisionTypeTile = 2,
  OCSCollisionTypeDrawShape = 3,
  OCSCollisionTypeSensor = 4,
  OCSCollisionTypeStaticEntity = 5
} OCSCollisionType;

extern SceneProto ChipmunkSceneProto;

#endif
