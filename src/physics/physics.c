#include <float.h>
#include <math.h>
#include "physics.h"

int Physics_init(void *self) {
    check_mem(self);
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

PhysPoint PhysPoint_scale(PhysPoint a, double b) {
    PhysPoint new = {a.x * b, a.y * b};
    return new;
}

double PhysPoint_dot(PhysPoint a, PhysPoint b) {
    return a.x * b.x + a.y * b.y;
}

double PhysPoint_cross(PhysPoint a, PhysPoint b) {
    return a.x * b.x - a.y * b.y;
}

PhysPoint PhysPoint_rotate(PhysPoint point, PhysPoint pivot,
        double angle_in_rads) {

    PhysPoint rotated = {
        point.x - pivot.x,
        point.y - pivot.y
    };

    double radians = angle_in_rads;
    double s = sin(radians);
    double c = cos(radians);

    double xnew = rotated.x * c - rotated.y * s;
    double ynew = rotated.x * s + rotated.y * c;

    rotated.x = xnew + pivot.x;
    rotated.y = ynew + pivot.y;

    return rotated;
}

PhysPoint PhysPoint_perp(PhysPoint a) {
    PhysPoint new = a;
    double tmp = new.y;
    new.y = -1 * new.x;
    new.x = tmp;
    return new;
}

PhysPoint PhysPoint_normalize(PhysPoint a) {
    PhysPoint new = a;
    if (a.x == 0.0 && a.y == 0.0) {
        return a;
    }
    double length = PhysPoint_magnitude(a);
    new.x /= length;
    new.y /= length;
    return new;
}

double PhysPoint_magnitude(PhysPoint a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

// ========================
// PhysProjection functions
// ========================
int PhysProjection_does_overlap(PhysProjection a, PhysProjection b) {
    return !(b.max < a.min || a.max < b.min);
}

double PhysProjection_get_overlap(PhysProjection a, PhysProjection b) {
    return (a.max < b.max ? a.max - b.min : b.max - a.min);
}

// =================
// PhysBox functions
// =================
PhysPoint PhysBox_center(PhysBox box) {
    PhysPoint diagonal = PhysPoint_subtract(box.br, box.tl);
    PhysPoint center = PhysPoint_add(box.tl, PhysPoint_scale(diagonal, 0.5));
    return center;
}

PhysPoint PhysBox_vertex(PhysBox box, unsigned int index) {
    assert(index <= 3);
    if (index == 0) return box.tl;
    if (index == 1) return box.tr;
    if (index == 2) return box.br;
    if (index == 3) return box.bl;

    PhysPoint no = {0,0};
    return no;
}

PhysBox PhysBox_rotate(PhysBox box, PhysPoint pivot, double angle_in_rads) {
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

void PhysBox_find_axes(PhysBox box, PhysPoint *axes) {
    for (int i = 0; i < 2; i++) {
        PhysPoint p1 = PhysBox_vertex(box, i);
        PhysPoint p2 = PhysBox_vertex(box, i + 1);
        PhysPoint edge = PhysPoint_subtract(p1, p2);
        PhysPoint perp = PhysPoint_perp(edge);
        PhysPoint normal = PhysPoint_normalize(perp);
        axes[i] = normal;
    }
}

PhysProjection PhysBox_project_onto(PhysBox box, PhysPoint axis) {
    double min = PhysPoint_dot(axis, PhysBox_vertex(box, 0));
    double max = min;
    int i = 0;
    for (i = 0; i < 4; i++) {
        double dot = PhysPoint_dot(axis, PhysBox_vertex(box, i));
        if (dot < min) min = dot;
        if (dot > max) max = dot;
    }
    PhysProjection proj = {min, max};
    return proj;
}

int PhysBox_collision(PhysBox a, PhysBox b, PhysPoint *mtv) {
    double overlap = FLT_MAX;
    PhysPoint smallest = {0, 0};
    PhysPoint axes_a[2];
    PhysBox_find_axes(a, axes_a);

    PhysPoint axes_b[2];
    PhysBox_find_axes(b, axes_b);
    int i = 0;
    for (i = 0; i < 2; i++) {
        PhysPoint axis = axes_a[i];
        PhysProjection p1 = PhysBox_project_onto(a, axis);
        PhysProjection p2 = PhysBox_project_onto(b, axis);
        int overlaps = PhysProjection_does_overlap(p1, p2);
        if (overlaps) {
            double o = PhysProjection_get_overlap(p1, p2);
            if (o < overlap) {
                overlap = o;
                smallest = axis;
            }
        } else {
            return 0;
        }
    }

    for (i = 0; i < 2; i++) {
        PhysPoint axis = axes_b[i];
        PhysProjection p1 = PhysBox_project_onto(a, axis);
        PhysProjection p2 = PhysBox_project_onto(b, axis);
        int overlaps = PhysProjection_does_overlap(p1, p2);
        if (overlaps) {
            double o = PhysProjection_get_overlap(p1, p2);
            if (o < overlap) {
                overlap = o;
                smallest = axis;
            }
        } else {
            return 0;
        }
    }

    if (mtv != NULL) {
        *mtv = PhysPoint_scale(smallest, overlap);
    }
    return 1;
}

PhysPoint PhysBox_poc(PhysBox a, PhysBox b) {
    PhysPoint closest = {0,0};

    int num_colliding_verts = 0;
    int i = 0;
    for (i = 0; i < 4; i++) {
        PhysPoint vertex = PhysBox_vertex(a, i);
        if (PhysBox_contains_point(b, vertex)) {
            closest.x = closest.x * num_colliding_verts + vertex.x;
            closest.y = closest.y * num_colliding_verts + vertex.y;
            num_colliding_verts++;
            closest.x /= num_colliding_verts;
            closest.y /= num_colliding_verts;
        }
    }
    return closest;
}

PhysBox PhysBox_bounding_box(PhysBox rect) {
    double min_x, max_x, min_y, max_y;

    int i = 0;
    for (i = 0; i < 4; i++) {
        PhysPoint point = PhysBox_vertex(rect, i);
        if (i == 0) {
            min_x = point.x;
            max_x = point.x;
            min_y = point.y;
            max_y = point.y;
        }
        if (point.x < min_x) min_x = point.x;
        if (point.y < min_y) min_y = point.y;
        if (point.x > max_x) max_x = point.x;
        if (point.y > max_y) max_y = point.y;
    }

    PhysPoint tl = {min_x, min_y};
    PhysPoint tr = {max_x, min_y};
    PhysPoint br = {max_x, max_y};
    PhysPoint bl = {min_x, max_y};

    PhysBox bounding = {tl, tr, br, bl};
    return bounding;
}

int PhysBox_contains_point(PhysBox box, PhysPoint point) {
    // TODO: Make more precise
    PhysBox bounding = PhysBox_bounding_box(box);
    return point.x >= bounding.tl.x && point.x <= bounding.tr.x &&
        point.y >= bounding.tl.y && point.y <= bounding.bl.y;
}

int PhysBox_is_equal(PhysBox *a, PhysBox *b) {
    if (a->tl.x == b->tl.x && a->tl.y == b->tl.y &&
            a->tr.x == b->tr.x && a->tr.y == b->tr.y &&
            a->br.x == b->br.x && a->br.y == b->br.y &&
            a->bl.x == b->bl.x && a->bl.y == b->bl.y) return 1;
    return 0;
}

PhysPoint PhysBox_cnormal_from_mtv(PhysBox normal_for, PhysBox against,
        PhysPoint mtv) {
    PhysPoint center = PhysBox_center(normal_for);
    PhysPoint other_center = PhysBox_center(against);

    PhysPoint direction = PhysPoint_subtract(other_center, center);
    double dot = PhysPoint_dot(mtv, direction);
    PhysPoint collision_normal = {};
    if (dot >= 0) {
        collision_normal = PhysPoint_normalize(
                PhysPoint_scale(mtv, -1));
    } else {
        collision_normal = PhysPoint_normalize(mtv);
    }

    return collision_normal;
}
