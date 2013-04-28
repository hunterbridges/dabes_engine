#ifndef __ortho_chipmunk_scene_h
#define __ortho_chipmunk_scene_h
#include "scene.h"

typedef enum {
  OCSCollisionTypeEntity = 1,
  OCSCollisionTypeTile = 2
} OCSCollisionType;

void OrthoChipmunkScene_start(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_stop(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_cleanup(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_update(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_render(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine);

extern SceneProto OrthoChipmunkSceneProto;

#endif