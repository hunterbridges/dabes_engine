#ifndef __ortho_chipmunk_scene_h
#define __ortho_chipmunk_scene_h
#include "scene.h"

typedef struct {
    Scene *scene;
    Engine *engine;
} OCSIterData;

typedef enum {
  OCSCollisionTypeEntity = 1,
  OCSCollisionTypeTile = 2,
  OCSCollisionTypeDrawShape = 3,
  OCSCollisionTypeSensor = 4
} OCSCollisionType;

extern SceneProto OrthoChipmunkSceneProto;

#endif
