#ifndef __ortho_chipmunk_scene_h
#define __ortho_chipmunk_scene_h
#include "scene.h"

typedef enum {
  OCSCollisionTypeEntity = 1,
  OCSCollisionTypeTile = 2
} OCSCollisionType;

int OrthoChipmunkScene_init(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_start(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_stop(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_destroy(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_update(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_render(struct Scene *scene, Engine *engine);
void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine);

extern SceneProto OrthoChipmunkSceneProto;

#endif