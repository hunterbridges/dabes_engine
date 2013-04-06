#ifndef __ortho_physics_scene_h
#define __ortho_physics_scene_h
#include "scene.h"

int OrthoPhysicsScene_create(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_start(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_stop(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_destroy(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_update(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_render(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_control(struct Scene *scene, Engine *engine);

extern SceneProto OrthoPhysicsSceneProto;

#endif
