#ifndef __vpoint_h
#define __vpoint_h
#include <chipmunk/chipmunk.h>
#include "../prefix.h"

typedef enum {
  VPointRelWithin = 0,
  VPointRelXLess = 1 << 0,
  VPointRelXMore = 1 << 1,
  VPointRelYLess = 1 << 2,
  VPointRelYMore = 1 << 3
} VPointRel;

typedef struct VPoint {
    float x;
    float y;
} VPoint;

static const VPoint VPointZero = {0,0};

VPoint VPoint_add(VPoint a, VPoint b);
VPoint VPoint_subtract(VPoint a, VPoint b);
double VPoint_angle(VPoint a, VPoint b);
VPoint VPoint_scale(VPoint a, double b);
double VPoint_dot(VPoint a, VPoint b);
double VPoint_cross(VPoint a, VPoint b);
VPoint VPoint_rotate(VPoint point, VPoint pivot, double angle_in_rads);
VPoint VPoint_perp(VPoint a);
VPoint VPoint_normalize(VPoint a);
double VPoint_magnitude(VPoint a);
VPointRel VPoint_rel(VPoint a, VPoint b);

static inline void VPoint_debug(VPoint point, char *msg) {
    debug("<%f, %f> %s", point.x, point.y, msg);
}

#endif
