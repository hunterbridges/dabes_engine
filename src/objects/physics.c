#include "physics.h"

int Physics_init(void *self) {
    check_mem(self);
    Physics *physics = self;
    return 1;
error:
    return 0;
}

Object PhysicsProto = {
    .init = Physics_init
};

// ===================
// PhysPoint functions
// ===================
PhysPoint PhysPoint_add(PhysPoint a, PhysPoint b) {
    PhysPoint new = {a.x + b.x, a.y + b.y};
    return new;
}

PhysPoint PhysPoint_subtract(PhysPoint a, PhysPoint b) {
    PhysPoint new = {a.x - b.x, a.y - b.y};
    return new;
}

PhysPoint PhysPoint_scale(PhysPoint a, float b) {
    PhysPoint new = {a.x * b, a.y * b};
    return new;
}

float PhysPoint_dot(PhysPoint a, PhysPoint b) {
    return a.x * b.x + a.y * b.y;
}

float PhysPoint_cross(PhysPoint a, PhysPoint b) {
    return a.x * b.x - a.y * b.y;
}

PhysPoint PhysPoint_rotate(PhysPoint point, PhysPoint pivot,
        float angle_in_rads) {

    PhysPoint rotated = {
        point.x - pivot.x,
        point.y - pivot.y
    };

    float radians = angle_in_rads;
    float s = sin(radians);
    float c = cos(radians);

    float xnew = rotated.x * c - rotated.y * s;
    float ynew = rotated.x * s + rotated.y * c;

    rotated.x = xnew + pivot.x;
    rotated.y = ynew + pivot.y;

    return rotated;
}

// =================
// PhysBox functions
// =================
PhysPoint PhysBox_center(PhysBox box) {
    PhysPoint diagonal = PhysPoint_subtract(box.br, box.tl);
    PhysPoint center = PhysPoint_add(box.tl, PhysPoint_scale(diagonal, 0.5));
    return center;
}

PhysPoint PhysBox_vertex(PhysBox box, uint index) {
    assert(index <= 3);
    if (index == 0) return box.tl;
    if (index == 1) return box.tr;
    if (index == 2) return box.br;
    if (index == 3) return box.bl;

    PhysPoint no = {0,0};
    return no;
}

PhysBox PhysBox_rotate(PhysBox box, PhysPoint pivot, float angle_in_rads) {
    PhysBox rotated = box;
    rotated.tl = PhysPoint_rotate(rotated.tl, pivot, angle_in_rads);
    rotated.tr = PhysPoint_rotate(rotated.tr, pivot, angle_in_rads);
    rotated.br = PhysPoint_rotate(rotated.br, pivot, angle_in_rads);
    rotated.bl = PhysPoint_rotate(rotated.bl, pivot, angle_in_rads);
    return rotated;
}

PhysBox PhysBox_move(PhysBox box, PhysPoint move) {
    PhysBox moved = box;
    moved.tl = PhysPoint_add(moved.tl, move);
    moved.tr = PhysPoint_add(moved.tr, move);
    moved.br = PhysPoint_add(moved.br, move);
    moved.bl = PhysPoint_add(moved.bl, move);
    return moved;
}
