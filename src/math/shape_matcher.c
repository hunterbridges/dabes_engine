#include "shape_matcher.h"

ShapeSegment ShapeSegment_make(float length, float angle_degrees) {
    ShapeSegment segment = {
        .length = length,
        .angle_degrees = angle_degrees
    };

    return segment;
}

////////////////////////////////////////////////////////////////////////////////

Shape *Shape_create(ShapeSegment *segments, int num_segments, const char *name)
{
    check(segments != NULL, "segments required");
    check(num_segments > 0, "Must provide more than 0 segments");

    Shape *shape = calloc(1, sizeof(Shape));
    check(shape != NULL, "Couldn't create shape");

    size_t seg_sz = num_segments * sizeof(ShapeSegment);
    ShapeSegment *my_segments = calloc(1, seg_sz);
    memcpy(my_segments, segments, seg_sz);

    shape->segments = my_segments;
    shape->num_segments = num_segments;

    shape->name = calloc(1, (strlen(name) + 1) * sizeof(char));
    strcpy(shape->name, name);

    return shape;
error:
    return NULL;
}

void Shape_destroy(Shape *shape) {
    check(shape != NULL, "Shape required");

    free(shape->segments);
    free(shape->name);
    free(shape);

    return;
error:
    return;
}

static float angle_wrap(float angle) {
    while (angle < 0) {
        angle += 360;
    }
    while (angle >= 360) {
        angle -= 360;
    }
    return angle;
}

void Shape_get_points(Shape *shape, VPoint start_point, float initial_length,
        float initial_angle_degrees, ShapeWinding winding, VPoint **points,
        int *num_points, VPoint *farthest_point) {
    check(shape != NULL, "Shape required");
    check(points != NULL, "**points required");
    check(num_points != NULL, "*num_points required");

    int num = shape->num_segments + 2;
    *num_points = num;

    VPoint *pts = calloc(num, sizeof(VPoint));
    int pt_idx = 0;
    pts[pt_idx++] = start_point;

    float angle_rads = initial_angle_degrees * M_PI / 180.f;
    VPoint second = VPoint_look_from(start_point, angle_rads, initial_length);
    pts[pt_idx++] = second;

    float prev_angle = initial_angle_degrees;
    float prev_angle_rads = angle_rads;
    VPoint prev_point = second;

    VPoint far_point = VPointZero;
    float far_point_dist = FLT_MIN;
    int i = 0;
    for (i = 0; i < shape->num_segments; i++) {
        ShapeSegment segment = shape->segments[i];

        float next_angle_delta = segment.angle_degrees * winding;
        float next_angle = angle_wrap(prev_angle + 180 + next_angle_delta);
        float next_angle_rads = next_angle * M_PI / 180.f;
        float next_length = segment.length * initial_length;

        VPoint next_point = VPoint_look_from(prev_point, next_angle_rads,
                                             next_length);
        float distance = VPoint_distance(start_point, next_point);
        if (distance > far_point_dist) {
            far_point_dist = distance;
            far_point = next_point;
        }
        pts[pt_idx++] = next_point;

        prev_angle = next_angle;
        prev_angle_rads = next_angle_rads;
        prev_point = next_point;
    }

    if (farthest_point) {
        *farthest_point = far_point;
    }

    *points = pts;

    return;
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////

ShapeMatcher *ShapeMatcher_create(Shape *shapes[], int num_shapes) {
    check(shapes != NULL, "Shapes required");
    check(num_shapes > 0, "More than 0 shapes required");

    ShapeMatcher *matcher = calloc(1, sizeof(ShapeMatcher));
    matcher->num_shapes = num_shapes;

    matcher->shapes = DArray_create(sizeof(Shape *), num_shapes);
    int i = 0;
    for (i = 0; i < num_shapes; i++) {
        Shape *shape = shapes[i];
        DArray_push(matcher->shapes, shape);
    }

    matcher->vertex_catch_tolerance = SHAPE_MATCHER_DEFAULT_VERTEX_CATCH;
    matcher->slop_tolerance = SHAPE_MATCHER_DEFAULT_SLOP;
    matcher->state = SHAPE_MATCHER_STATE_NEW;

    return matcher;
error:
    return NULL;
}

void ShapeMatcher_clear_staged_point(ShapeMatcher *matcher) {
    if (matcher->staged_point) {
        free(matcher->staged_point);
    }

    matcher->staged_point = NULL;
}

void ShapeMatcher_clear_points(ShapeMatcher *matcher) {
    if (matcher->points) {
        DArray_clear_destroy(matcher->points);
        matcher->points = NULL;
    }
}

void ShapeMatcher_clear_potential_shapes(ShapeMatcher *matcher) {
    if (matcher->potential_shapes) {
        BSTree_destroy(matcher->potential_shapes);
        matcher->potential_shapes = NULL;
    }
}

void ShapeMatcher_init_potential_shapes(ShapeMatcher *matcher) {
    matcher->potential_shapes = BSTree_create((BSTree_compare)strcmp);

    int i = 0;
    for (i = 0; i < DArray_count(matcher->shapes); i++) {
        Shape *shape = DArray_get(matcher->shapes, i);
        BSTree_set(matcher->potential_shapes, shape->name, shape);
    }
}

void ShapeMatcher_set_state(ShapeMatcher *matcher, ShapeMatcherState state) {
    matcher->state = state;
}

void ShapeMatcher_reset(ShapeMatcher *matcher) {
    matcher->initial_segment_length = 0.f;
    matcher->initial_segment_angle = 0.f;
    matcher->intended_convex_winding = SHAPE_WINDING_AMBIGUOUS;
    ShapeMatcher_set_state(matcher, SHAPE_MATCHER_STATE_NEW);

    ShapeMatcher_clear_points(matcher);
    ShapeMatcher_clear_staged_point(matcher);
    ShapeMatcher_clear_potential_shapes(matcher);
}

int ShapeMatcher_start(ShapeMatcher *matcher, VPoint point) {
    check(matcher != NULL, "No matcher to start");
    assert(matcher->state != SHAPE_MATCHER_STATE_RUNNING);

    ShapeMatcher_reset(matcher);
    ShapeMatcher_init_potential_shapes(matcher);

    VPoint *ppoint = calloc(1, sizeof(VPoint));
    *ppoint = point;

    // TODO init matcher->matched_points

    matcher->points = DArray_create(sizeof(VPoint), 8);
    DArray_push(matcher->points, ppoint);

    ShapeMatcher_set_state(matcher, SHAPE_MATCHER_STATE_RUNNING);

    return 1;
error:
    return 0;
}

int ShapeMatcher_stage_point(ShapeMatcher *matcher, VPoint point) {
    check(matcher != NULL, "No matcher to stage point");

    ShapeMatcher_clear_staged_point(matcher);
    matcher->staged_point = calloc(1, sizeof(VPoint));
    *matcher->staged_point = point;

    VPoint *last = DArray_last(matcher->points);

    int num_points = DArray_count(matcher->points);
    if (num_points == 1) {
        matcher->initial_segment_angle =
            VPoint_angle(*last, point) * 180.f / M_PI;
        matcher->initial_segment_length = VPoint_distance(*last, point);
    } else if (num_points == 2) {
        // TODO Calculate winding
        // TODO Prepare to match points
    }

    return 1;
error:
    return 0;
}

int ShapeMatcher_commit_point(ShapeMatcher *matcher) {
    check(matcher != NULL, "No matcher to commit");

    DArray_push(matcher->points, matcher->staged_point);
    matcher->staged_point = NULL;

    if (matcher->intended_convex_winding == SHAPE_WINDING_AMBIGUOUS) return 1;

    // TODO Widdle down shapes

    return 1;
error:
    return 0;
}

int ShapeMatcher_end(ShapeMatcher *matcher) {
    check(matcher != NULL, "No matcher to end");
    if (matcher->state == SHAPE_MATCHER_STATE_ENDED) return 0;

    assert(matcher->state == SHAPE_MATCHER_STATE_RUNNING);

    // will end delegate was here

    // TODO check if we matched a shape

    return 1;
error:
    return 0;
}
