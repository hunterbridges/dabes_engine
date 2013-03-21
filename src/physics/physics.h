#ifndef __physics_h
#define __physics_h
#include "../prefix.h"

typedef struct Physics {
    Object proto;
} Physics;

int Physics_init(void *self);

extern Object PhysicsProto;

typedef struct PhysPoint {
    double x;
    double y;
} PhysPoint;

PhysPoint PhysPoint_add(PhysPoint a, PhysPoint b);
PhysPoint PhysPoint_subtract(PhysPoint a, PhysPoint b);
PhysPoint PhysPoint_scale(PhysPoint a, double b);
double PhysPoint_dot(PhysPoint a, PhysPoint b);
double PhysPoint_cross(PhysPoint a, PhysPoint b);
PhysPoint PhysPoint_rotate(PhysPoint point, PhysPoint pivot, double angle_in_rads);
PhysPoint PhysPoint_perp(PhysPoint a);
PhysPoint PhysPoint_normalize(PhysPoint a);
double PhysPoint_magnitude(PhysPoint a);

typedef struct PhysProjection {
    double min;
    double max;
} PhysProjection;

int PhysProjection_does_overlap(PhysProjection a, PhysProjection b);
double PhysProjection_get_overlap(PhysProjection a, PhysProjection b);

typedef struct PhysBox {
    PhysPoint tl;
    PhysPoint tr;
    PhysPoint br;
    PhysPoint bl;
} PhysBox;

PhysPoint PhysBox_center(PhysBox box);
PhysPoint PhysBox_vertex(PhysBox box, unsigned int index);
PhysBox PhysBox_rotate(PhysBox box, PhysPoint pivot, double angle_in_rads);
PhysBox PhysBox_move(PhysBox box, PhysPoint move);
PhysProjection PhysBox_project_onto(PhysBox box, PhysPoint axis);
void PhysBox_find_axes(PhysBox box, PhysPoint *axes);
int PhysBox_collision(PhysBox a, PhysBox b, PhysPoint *mtv);
PhysPoint PhysBox_poc(PhysBox a, PhysBox b);
int PhysBox_contains_point(PhysBox box, PhysPoint point);
PhysBox PhysBox_bounding_box(PhysBox rect);
int PhysBox_is_equal(PhysBox *a, PhysBox *b);
PhysPoint PhysBox_cnormal_from_mtv(PhysBox normal_for, PhysBox against,
        PhysPoint mtv);

static inline void PhysPoint_debug(PhysPoint point, char *msg) {
    debug("<%f, %f> %s", point.x, point.y, msg);
}

#endif
