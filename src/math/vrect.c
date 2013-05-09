#include <stdlib.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include "vrect.h"

// ========================
// VProjection functions
// ========================
int VProjection_does_overlap(VProjection a, VProjection b) {
    return !(b.max < a.min || a.max < b.min);
}

double VProjection_get_overlap(VProjection a, VProjection b) {
    return (a.max < b.max ? a.max - b.min : b.max - a.min);
}

// =================
// VRect functions
// =================

VRect VRect_from_xywh(double x, double y, double w, double h) {
    VPoint tl = { x, y };
    VPoint tr = { x + w, y };
    VPoint bl = { x, y + h };
    VPoint br = { x + w, y + h };
    VRect rect = {
        .tl = tl,
        .tr = tr,
        .bl = bl,
        .br = br
    };
    return rect;
}

VRect VRect_inset(VRect rect, VRectInset inset) {
  double rot = VPoint_angle(rect.tl, rect.tr);
  VRect new = VRect_rotate(rect, VRect_center(rect), -rot);

  new.tl.x += inset.left;
  new.tl.y += inset.top;

  new.tr.x -= inset.right;
  new.tr.y += inset.top;

  new.bl.x += inset.left;
  new.bl.y -= inset.bottom;

  new.br.x -= inset.right;
  new.br.y -= inset.bottom;

  new = VRect_rotate(new, VRect_center(new), rot);
  return new;
}

VRect VRect_round_out(VRect rect) {
  rect.tl.x = floor(rect.tl.x);
  rect.tl.y = floor(rect.tl.y);

  rect.tr.x = ceil(rect.tr.x);
  rect.tr.y = floor(rect.tl.y);

  rect.bl.x = floor(rect.bl.x);
  rect.bl.y = ceil(rect.bl.y);

  rect.br.x = ceil(rect.br.x);
  rect.br.y = ceil(rect.br.y);

  return rect;
}

float VRect_width(VRect rect) {
    return VPoint_magnitude(VPoint_subtract(rect.tr, rect.tl));
}

float VRect_height(VRect rect) {
    return VPoint_magnitude(VPoint_subtract(rect.bl, rect.tl));
}

VPoint VRect_center(VRect box) {
    VPoint diagonal = VPoint_subtract(box.br, box.tl);
    VPoint center = VPoint_add(box.tl, VPoint_scale(diagonal, 0.5));
    return center;
}

VPoint VRect_vertex(VRect box, unsigned int index) {
    assert(index <= 3);
    if (index == 0) return box.tl;
    if (index == 1) return box.tr;
    if (index == 2) return box.br;
    if (index == 3) return box.bl;

    VPoint no = {0,0};
    return no;
}

VRect VRect_rotate(VRect box, VPoint pivot, double angle_in_rads) {
    VRect rotated = box;
    rotated.tl = VPoint_rotate(rotated.tl, pivot, angle_in_rads);
    rotated.tr = VPoint_rotate(rotated.tr, pivot, angle_in_rads);
    rotated.br = VPoint_rotate(rotated.br, pivot, angle_in_rads);
    rotated.bl = VPoint_rotate(rotated.bl, pivot, angle_in_rads);
    return rotated;
}

VRect VRect_move(VRect box, VPoint move) {
    VRect moved = box;
    moved.tl = VPoint_add(moved.tl, move);
    moved.tr = VPoint_add(moved.tr, move);
    moved.br = VPoint_add(moved.br, move);
    moved.bl = VPoint_add(moved.bl, move);
    return moved;
}

VRect VRect_scale(VRect box, double scale) {
    VRect scaled = box;
    VPoint center = VRect_center(scaled);
    scaled = VRect_move(scaled, VPoint_scale(center, -1));
    scaled.tl = VPoint_scale(scaled.tl, scale);
    scaled.tr = VPoint_scale(scaled.tr, scale);
    scaled.bl = VPoint_scale(scaled.bl, scale);
    scaled.br = VPoint_scale(scaled.br, scale);
    scaled = VRect_move(scaled, VPoint_scale(center, scale));
    return scaled;
}

void VRect_find_axes(VRect box, VPoint *axes) {
    for (int i = 0; i < 2; i++) {
        VPoint p1 = VRect_vertex(box, i);
        VPoint p2 = VRect_vertex(box, i + 1);
        VPoint edge = VPoint_subtract(p1, p2);
        VPoint perp = VPoint_perp(edge);
        VPoint normal = VPoint_normalize(perp);
        axes[i] = normal;
    }
}

VProjection VRect_project_onto(VRect box, VPoint axis) {
    double min = VPoint_dot(axis, VRect_vertex(box, 0));
    double max = min;
    int i = 0;
    for (i = 0; i < 4; i++) {
        double dot = VPoint_dot(axis, VRect_vertex(box, i));
        if (dot < min) min = dot;
        if (dot > max) max = dot;
    }
    VProjection proj = {min, max};
    return proj;
}

int VRect_collision(VRect a, VRect b, VPoint *mtv) {
    double overlap = FLT_MAX;
    VPoint smallest = {0, 0};
    VPoint axes_a[2];
    VRect_find_axes(a, axes_a);

    VPoint axes_b[2];
    VRect_find_axes(b, axes_b);
    int i = 0;
    for (i = 0; i < 2; i++) {
        VPoint axis = axes_a[i];
        VProjection p1 = VRect_project_onto(a, axis);
        VProjection p2 = VRect_project_onto(b, axis);
        int overlaps = VProjection_does_overlap(p1, p2);
        if (overlaps) {
            double o = VProjection_get_overlap(p1, p2);
            if (o < overlap) {
                overlap = o;
                smallest = axis;
            }
        } else {
            return 0;
        }
    }

    for (i = 0; i < 2; i++) {
        VPoint axis = axes_b[i];
        VProjection p1 = VRect_project_onto(a, axis);
        VProjection p2 = VRect_project_onto(b, axis);
        int overlaps = VProjection_does_overlap(p1, p2);
        if (overlaps) {
            double o = VProjection_get_overlap(p1, p2);
            if (o < overlap) {
                overlap = o;
                smallest = axis;
            }
        } else {
            return 0;
        }
    }

    if (mtv != NULL) {
        *mtv = VPoint_scale(smallest, overlap);
    }
    return 1;
}

VPoint VRect_poc(VRect a, VRect b) {
    VPoint closest = {0,0};

    int num_colliding_verts = 0;
    int i = 0;
    for (i = 0; i < 4; i++) {
        VPoint vertex = VRect_vertex(a, i);
        if (VRect_contains_point(b, vertex)) {
            closest.x = closest.x * num_colliding_verts + vertex.x;
            closest.y = closest.y * num_colliding_verts + vertex.y;
            num_colliding_verts++;
            closest.x /= num_colliding_verts;
            closest.y /= num_colliding_verts;
        }
    }
    return closest;
}

VRect VRect_bounding_box(VRect rect) {
    float min_x, max_x, min_y, max_y;

    int i = 0;
    for (i = 0; i < 4; i++) {
        VPoint point = VRect_vertex(rect, i);
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

    VPoint tl = {min_x, min_y};
    VPoint tr = {max_x, min_y};
    VPoint br = {max_x, max_y};
    VPoint bl = {min_x, max_y};

    VRect bounding = {tl, tr, br, bl};
    return bounding;
}

int VRect_contains_point(VRect box, VPoint point) {
    // TODO: Make more precise
    VRect bounding = VRect_bounding_box(box);
    return point.x >= bounding.tl.x && point.x <= bounding.tr.x &&
        point.y >= bounding.tl.y && point.y <= bounding.bl.y;
}

int VRect_contains_rect(VRect outer, VRect inner) {
  int i = 0;
  int contains = 1;
  for (i = 0; i < 4; i++) {
    VPoint test = VRect_vertex(inner, i);
    contains = contains && VRect_contains_point(outer, test);
    if (!contains) break;
  }
  return contains;
}

int VRect_is_equal(VRect *a, VRect *b) {
    if (a->tl.x == b->tl.x && a->tl.y == b->tl.y &&
            a->tr.x == b->tr.x && a->tr.y == b->tr.y &&
            a->br.x == b->br.x && a->br.y == b->br.y &&
            a->bl.x == b->bl.x && a->bl.y == b->bl.y) return 1;
    return 0;
}

VPoint VRect_cnormal_from_mtv(VRect normal_for, VRect against,
        VPoint mtv) {
    VPoint center = VRect_center(normal_for);
    VPoint other_center = VRect_center(against);

    VPoint direction = VPoint_subtract(other_center, center);
    double dot = VPoint_dot(mtv, direction);
    VPoint collision_normal;
    if (dot >= 0) {
        collision_normal = VPoint_normalize(
                VPoint_scale(mtv, -1));
    } else {
        collision_normal = VPoint_normalize(mtv);
    }

    return collision_normal;
}

VPointRel VPoint_rect_rel(VPoint point, VRect rect) {
    int i = 0;
    unsigned short int all = 15;
    VPointRel rel = all;
    for (i = 0; i < 4; i++) {
      VPoint vertex = VRect_vertex(rect, i);
      rel &= VPoint_rel(point, vertex);
    }

    return rel;
}

#ifdef DABES_SDL
VRect VRect_from_SDL_Rect(SDL_Rect rect) {
    return VRect_from_xywh(rect.x, rect.y, rect.w, rect.h);
}
#endif
