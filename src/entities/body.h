#ifndef __body_h
#define __body_h
#include <chipmunk/chipmunk.h>
#include <lcthw/list.h>
#include "../prefix.h"
#include "../math/vpoint.h"
#include "../physics/fixture.h"

struct Entity;
struct Engine;
struct Scene;
typedef struct {
  struct Engine *engine;
  struct Entity *entity;
  struct Scene *scene;
  int on_ground;
} BodyStateData;

struct Body;
struct Sensor;
typedef struct BodyProto {
    int (*init)(struct Body *body, float w, float h, float mass,
            int can_rotate);
    void (*cleanup)(struct Body *body);

    VRect (*gfx_rect)(struct Body *body, float pixels_per_meter, int rotate);
    VPoint (*gfx_center)(struct Body *body, float pixels_per_meter);

    void (*add_sensor)(struct Body *body, struct Sensor *sensor);
    void (*remove_sensor)(struct Body *body, struct Sensor *sensor);

    void (*apply_force)(struct Body *body, VPoint force, VPoint offset);
    void (*set_hit_box)(struct Body *body, float w, float h, VPoint offset);

    VPoint (*get_pos)(struct Body *body);
    void (*set_pos)(struct Body *body, VPoint pos);
    VPoint (*get_velo)(struct Body *body);
    void (*set_velo)(struct Body *body, VPoint velo);
    VPoint (*get_force)(struct Body *body);
    void (*set_force)(struct Body *body, VPoint force);
    float (*get_angle)(struct Body *body);
    void (*set_angle)(struct Body *body, float angle);
    float (*get_friction)(struct Body *body);
    void (*set_friction)(struct Body *body, float friction);
    float (*get_elasticity)(struct Body *body);
    void (*set_elasticity)(struct Body *body, float elasticity);
    float (*get_mass)(struct Body *body);
    void (*set_mass)(struct Body *body, float mass);
    int (*get_can_rotate)(struct Body *body);
    void (*set_can_rotate)(struct Body *body, int can_rotate);
    int (*get_is_rogue)(struct Body *body);
    void (*set_is_rogue)(struct Body *body, int is_rogue);
    int (*get_is_static)(struct Body *body);
    void (*set_is_static)(struct Body *body, int is_static);
    int (*get_collision_layers)(struct Body *body);
    void (*set_collision_layers)(struct Body *body, dab_uint32 collision_layers);

} BodyProto;

typedef struct Body {
    BodyProto proto;
    BodyStateData state;

    Fixture *fixture;

    cpSpace *cp_space;
    cpShape *cp_shape;
    cpBody *cp_body;

    dab_uint32 collision_layers;
    int is_rogue;
    int is_static;
    int can_rotate;
    float mass;
    float w;
    float h;

    VPoint draw_offset;

    void *context;

    List *sensors;
} Body;

Body *Body_create(BodyProto proto, float w, float h, float mass,
        int can_rotate);
void Body_destroy(Body *body);
void Body_add_sensor(Body *body, struct Sensor *sensor);
void Body_remove_sensor(Body *body, struct Sensor *sensor);

#endif
