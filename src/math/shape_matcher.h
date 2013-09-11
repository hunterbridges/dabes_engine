#ifndef __shape_matcher_h
#define __shape_matcher_h
#include <lcthw/darray.h>
#include <lcthw/bstree.h>
#include <lcthw/hashmap.h>
#include "vpoint.h"

typedef struct ShapeSegment {
    float length;
    float angle_degrees;
} ShapeSegment;

ShapeSegment ShapeSegment_make(float length, float angle_degrees);

////////////////////////////////////////////////////////////////////////////////

typedef enum {
    SHAPE_WINDING_COUNTERCLOCKWISE = 1,
    SHAPE_WINDING_AMBIGUOUS = 0,
    SHAPE_WINDING_CLOCKWISE = -1
} ShapeWinding;

typedef struct Shape {
    ShapeSegment *segments;
    int num_segments;
    char *name;
} Shape;

Shape *Shape_create(ShapeSegment *segments, int num_segments, const char *name);
void Shape_destroy(Shape *shape);
VPath *Shape_get_path(Shape *shape, VPoint start_point, float initial_length,
    float initial_angle_degrees, ShapeWinding winding, VPoint *farthest_point);

////////////////////////////////////////////////////////////////////////////////

typedef struct MatchedPoint {
    int index;
    float distance;
} MatchedPoint;

typedef struct PotentialShape {
    Shape *shape;
    DArray *matched_points;
    VPath *path;
    VPoint farthest;
} PotentialShape;

PotentialShape *PotentialShape_create(Shape *shape);
void PotentialShape_destroy(PotentialShape *pshape);

////////////////////////////////////////////////////////////////////////////////

static float SHAPE_MATCHER_DEFAULT_VERTEX_CATCH = 0.2;
static float SHAPE_MATCHER_DEFAULT_SLOP = 1.1;

typedef enum {
    SHAPE_MATCHER_STATE_NEW = 0,
    SHAPE_MATCHER_STATE_RUNNING = 1,
    SHAPE_MATCHER_STATE_ENDED = 2
} ShapeMatcherState;

typedef struct ShapeMatcher {
    DArray *shapes;
    int num_shapes;
    ShapeMatcherState state;

    float vertex_catch_tolerance;
    float slop_tolerance;

    float initial_segment_angle;
    float initial_segment_length;
    ShapeWinding intended_convex_winding;

    DArray *points;
    VPoint *staged_point;
    BSTree *potential_shapes;

    PotentialShape *matched_shape;
} ShapeMatcher;

ShapeMatcher *ShapeMatcher_create(Shape *shapes[], int num_shapes);
void ShapeMatcher_reset(ShapeMatcher *matcher);
int ShapeMatcher_start(ShapeMatcher *matcher, VPoint point);
int ShapeMatcher_stage_point(ShapeMatcher *matcher, VPoint point);
int ShapeMatcher_commit_point(ShapeMatcher *matcher);
int ShapeMatcher_end(ShapeMatcher *matcher);

#endif
