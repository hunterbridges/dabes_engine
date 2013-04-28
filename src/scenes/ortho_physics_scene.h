#ifndef __ortho_physics_scene_h
#define __ortho_physics_scene_h
#include "scene.h"

void OrthoPhysicsScene_start(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_stop(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_cleanup(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_update(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_render(struct Scene *scene, Engine *engine);
void OrthoPhysicsScene_control(struct Scene *scene, Engine *engine);

extern SceneProto OrthoPhysicsSceneProto;

#endif
