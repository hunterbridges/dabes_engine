#ifndef __body_h
#define __body_h
#include <chipmunk/chipmunk.h>
#include <lcthw/list.h>
#include "../prefix.h"
#include "../math/vpoint.h"
#include "../physics/fixture.h"

typedef struct {
  void *engine;
  void *entity;
  void *scene;
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
    float (*get_mass)(struct Body *body);
    void (*set_mass)(struct Body *body, float mass);
    int (*get_can_rotate)(struct Body *body);
    void (*set_can_rotate)(struct Body *body, int can_rotate);
} BodyProto;

typedef struct Body {
    BodyProto proto;
    BodyStateData state;

    Fixture *fixture;

    cpSpace *cp_space;
    cpShape *cp_shape;
    cpBody *cp_body;

    int can_rotate;
    float mass;
    float w;
    float h;

    VPoint draw_offset;

    List *sensors;
} Body;

Body *Body_create(BodyProto proto, float w, float h, float mass,
        int can_rotate);
void Body_destroy(Body *body);
void Body_add_sensor(Body *body, struct Sensor *sensor);
void Body_remove_sensor(Body *body, struct Sensor *sensor);

#endif
