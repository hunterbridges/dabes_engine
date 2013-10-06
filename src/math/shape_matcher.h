#ifndef __shape_matcher_h
#define __shape_matcher_h
#include <lcthw/darray.h>
#include <lcthw/bstree.h>
#include <lcthw/hashmap.h>
#include "vcircle.h"
#include "vpoint.h"
#include "vmatrix.h"

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

extern const float SHAPE_MATCHER_DEFAULT_VERTEX_CATCH;
extern const float SHAPE_MATCHER_DEFAULT_SLOP;

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
    DArray *marked_shape_keys;

    PotentialShape *matched_shape;

    int debug_shapes;
    VVector4 debug_shape_color;
    int debug_shape_width;
    float debug_shape_z;

    VVector4 dot_color;
    int dot_width;
    float dot_z;
} ShapeMatcher;

struct Engine;

ShapeMatcher *ShapeMatcher_create(Shape *shapes[], int num_shapes);
void ShapeMatcher_destroy(ShapeMatcher *matcher);
void ShapeMatcher_reset(ShapeMatcher *matcher, struct Engine *engine);
int ShapeMatcher_start(ShapeMatcher *matcher, VPoint point,
                       struct Engine *engine);
int ShapeMatcher_stage_point(ShapeMatcher *matcher, VPoint point);
int ShapeMatcher_commit_point(ShapeMatcher *matcher, struct Engine *engine);
int ShapeMatcher_end(ShapeMatcher *matcher, struct Engine *engine);
void ShapeMatcher_get_potential_shape_paths(ShapeMatcher *matcher,
    VPath ***paths, int *num_paths);
void ShapeMatcher_get_connect_dots(ShapeMatcher *matcher, VCircle **circles,
    int *num_circles);

#endif
