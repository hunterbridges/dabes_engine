#include <math.h>
#include <stdio.h>
#include "vpoint.h"
#include "../util.h"

// ===================
// VPoint functions
// ===================
VPoint VPoint_add(VPoint a, VPoint b) {
    VPoint new = {a.x + b.x, a.y + b.y};
    return new;
}

VPoint VPoint_subtract(VPoint a, VPoint b) {
    VPoint new = {a.x - b.x, a.y - b.y};
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

VPointRel VPoint_rel(VPoint a, VPoint b) {
    VPointRel rel = VPointRelWithin;
    if (fequal(a.x, b.x) && fequal(a.y, b.y)) return rel;
    if (a.x < b.x) rel |= VPointRelXLess;
    if (a.x > b.x) rel |= VPointRelXMore;
    if (a.y < b.y) rel |= VPointRelYLess;
    if (a.y > b.y) rel |= VPointRelYMore;
    return rel;
}
