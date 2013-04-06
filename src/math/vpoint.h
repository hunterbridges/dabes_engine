#ifndef __vpoint_h
#define __vpoint_h

typedef enum {
  VRectPointRelWithin = 0,
  VRectPointRelXLess = 1 << 0,
  VRectPointRelXMore = 1 << 1,
  VRectPointRelYLess = 1 << 2,
  VRectPointRelYMore = 1 << 3
} VPointRel;

typedef struct VPoint {
  double x;
  double y;
} VPoint;

static const VPoint VPointZero = {0,0};

VPointRel VPoint_rel(VPoint a, VPoint b);

VPoint VPoint_add(VPoint a, VPoint b);
VPoint VPoint_subtract(VPoint a, VPoint b);
VPoint VPoint_scale(VPoint a, double b);
double VPoint_dot(VPoint a, VPoint b);
double VPoint_cross(VPoint a, VPoint b);
VPoint VPoint_rotate(VPoint point, VPoint pivot, double angle_in_rads);
VPoint VPoint_perp(VPoint a);
VPoint VPoint_normalize(VPoint a);
double VPoint_magnitude(VPoint a);

static inline void VPoint_debug(VPoint point, char *msg) {
    debug("<%f, %f> %s", point.x, point.y, msg);
}

#endif
