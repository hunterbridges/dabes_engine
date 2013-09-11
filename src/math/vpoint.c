#include <math.h>
#include <stdio.h>
#include "vpoint.h"
#include "../util.h"

// ===================
// VPoint functions
// ===================
VPoint VPoint_make(float x, float y) {
    VPoint new = {x, y};
    return new;
}

VPoint VPoint_add(VPoint a, VPoint b) {
    VPoint new = {a.x + b.x, a.y + b.y};
    return new;
}

VPoint VPoint_subtract(VPoint a, VPoint b) {
    VPoint new = {a.x - b.x, a.y - b.y};
    return new;
}

VPoint VPoint_multiply(VPoint a, VPoint b) {
    VPoint new = {a.x * b.x, a.y * b.y};
    return new;
}

double VPoint_angle(VPoint a, VPoint b) {
    VPoint d = VPoint_subtract(b, a);
    return atan2(d.y, d.x);
}

VPoint VPoint_scale(VPoint a, double b) {
    VPoint new = {a.x * b, a.y * b};
    return new;
}

double VPoint_dot(VPoint a, VPoint b) {
    return a.x * b.x + a.y * b.y;
}

double VPoint_cross(VPoint a, VPoint b) {
    return a.x * b.x - a.y * b.y;
}

VPoint VPoint_mid(VPoint a, VPoint b) {
    VPoint angle = VPoint_subtract(b, a);
    VPoint scaled = VPoint_scale(angle, 0.5);
    return VPoint_add(a, scaled);
}

VPoint VPoint_rotate(VPoint point, VPoint pivot,
        double angle_in_rads) {

    VPoint rotated = {
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

VPoint VPoint_perp(VPoint a) {
    VPoint new = a;
    double tmp = new.y;
    new.y = -1 * new.x;
    new.x = tmp;
    return new;
}

VPoint VPoint_normalize(VPoint a) {
    VPoint new = a;
    if (a.x == 0.0 && a.y == 0.0) {
        return a;
    }
    double length = VPoint_magnitude(a);
    new.x /= length;
    new.y /= length;
    return new;
}

double VPoint_magnitude(VPoint a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

double VPoint_distance(VPoint a, VPoint b) {
    return VPoint_magnitude(VPoint_subtract(b, a));
}

VPoint VPoint_look_from(VPoint start_point, float angle_rads, float mag) {
    VPoint translation = {
        cosf(angle_rads) * mag,
        sinf(angle_rads) * mag
    };

    VPoint next = VPoint_add(start_point, translation);

    return next;
}

VPointRel VPoint_rel(VPoint a, VPoint b) {
    VPointRel rel = VPointRelWithin;
    if (fequal(a.x, b.x) && fequal(a.y, b.y)) return rel;
    if (a.x < b.x) rel |= VPointRelXLess;
    if (a.x > b.x) rel |= VPointRelXMore;
    if (a.y < b.y) rel |= VPointRelYLess;
    if (a.y > b.y) rel |= VPointRelYMore;
    return rel;
}

////////////////////////////////////////////////////////////////////////////////

VPath *VPath_create(VPoint *points, int num_points) {
    check(points != NULL, "Points required");
    check(num_points > 0, "More than 0 points required");
    VPath *path = malloc(sizeof(VPath) + num_points * sizeof(VPoint));

    int i = 0;
    for (i = 0; i < num_points; i++) {
        path->points[i] = points[i];
    }

    return path;
error:
    return NULL;
}

void VPath_destroy(VPath *path) {
    check(path != NULL, "No path to destroy");
    free(path);
    return;
error:
    return;
}

void VPath_translate(VPath *path, VPoint trans) {
    check(path != NULL, "No path to translate");

    int i = 0;
    for (i = 0; i < path->num_points; i++) {
        VPoint pt = path->points[i];
        path->points[i] = VPoint_add(pt, trans);
    }

    return;
error:
    return;
}
