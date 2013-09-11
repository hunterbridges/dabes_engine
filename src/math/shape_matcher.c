#include "shape_matcher.h"
#include "vpolygon.h"

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

VPath *Shape_get_path(Shape *shape, VPoint start_point, float initial_length,
        float initial_angle_degrees, ShapeWinding winding, VPoint *farthest_point) {
    check(shape != NULL, "Shape required");

    int num = shape->num_segments + 2;

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

    return VPath_create(pts, num);
error:
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

PotentialShape *PotentialShape_create(Shape *shape) {
    check(shape != NULL, "Shape required to create PotentialShape");

    PotentialShape *pshape = calloc(1, sizeof(PotentialShape));
    pshape->shape = shape;
    pshape->matched_points = DArray_create(sizeof(MatchedPoint *),
            shape->num_segments + 2);

    MatchedPoint *one = malloc(sizeof(MatchedPoint));
    one->index = 0;
    one->distance = 0;

    MatchedPoint *two = malloc(sizeof(MatchedPoint));
    two->index = 1;
    two->distance = 0;

    DArray_push(pshape->matched_points, one);
    DArray_push(pshape->matched_points, two);

    return pshape;
error:
    return NULL;
}

void PotentialShape_destroy(PotentialShape *pshape) {
    check(pshape != NULL, "No PotentialShape to destroy");

    DArray_clear_destroy(pshape->matched_points);
    free(pshape);

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

    VVector4 dot_color = {.raw = {0.5, 0.5, 0.5, 0.5}};
    matcher->dot_color = dot_color;
    matcher->dot_width = 2;

    VVector4 debug_shape_color = {.raw = {0, 1.0, 1.0, 1.0}};
    matcher->debug_shape_color = debug_shape_color;
    matcher->debug_shape_width = 5;

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

int potential_shapes_clear_cb(BSTreeNode *node, void *UNUSED(ctx)) {
    PotentialShape *pshape = node->data;
    PotentialShape_destroy(pshape);
    return 1;
}

void ShapeMatcher_clear_potential_shapes(ShapeMatcher *matcher) {
    if (matcher->potential_shapes) {
        BSTree_traverse(matcher->potential_shapes, potential_shapes_clear_cb,
                NULL);
        BSTree_destroy(matcher->potential_shapes);
        matcher->potential_shapes = NULL;
    }
}

void ShapeMatcher_init_potential_shapes(ShapeMatcher *matcher) {
    matcher->potential_shapes = BSTree_create((BSTree_compare)strcmp);

    int i = 0;
    for (i = 0; i < DArray_count(matcher->shapes); i++) {
        Shape *shape = DArray_get(matcher->shapes, i);
        BSTree_set(matcher->potential_shapes, shape->name,
                PotentialShape_create(shape));
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

    matcher->matched_shape = NULL;
    ShapeMatcher_init_potential_shapes(matcher);

    VPoint *ppoint = calloc(1, sizeof(VPoint));
    *ppoint = point;

    matcher->points = DArray_create(sizeof(VPoint), 8);
    DArray_push(matcher->points, ppoint);

    ShapeMatcher_set_state(matcher, SHAPE_MATCHER_STATE_RUNNING);

    return 1;
error:
    return 0;
}

VPath *ShapeMatcher_staged_path(ShapeMatcher *matcher) {
    int pt_count = DArray_count(matcher->points);
    int staged_d = matcher->staged_point ? 1 : 0;
    int staged_count = pt_count + staged_d;
    VPoint pts[staged_count];

    int i = 0;
    for (i = 0; i < staged_count; i++) {
        if (i < pt_count) {
            VPoint *pt = DArray_get(matcher->points, i);
            pts[i] = *pt;
        } else {
            pts[i] = *matcher->staged_point;
        }
    }

    return VPath_create(pts, staged_count);
}

ShapeWinding ShapeMatcher_calc_winding(ShapeMatcher *matcher) {
    VPath *spath = ShapeMatcher_staged_path(matcher);
    check(spath != NULL, "Couldn't get staged path for winding");
    check(spath->num_points >= 2, "Not enough points to get matcher winding");

    int i = 0;
    float xavg = 0;
    float yavg = 0;
    for (i = 0; i < spath->num_points; i++) {
        VPoint pt = spath->points[i];
        xavg += pt.x;
        yavg += pt.y;
    }
    xavg /= spath->num_points;
    yavg /= spath->num_points;

    VPoint corr = VPoint_make(-xavg, -yavg);
    VPath_translate(spath, corr);

    VPolygon *poly = VPolygon_create(spath->num_points, spath->points);

    ShapeWinding winding = (VPolygon_is_clockwise(poly) ?
                            SHAPE_WINDING_CLOCKWISE :
                            SHAPE_WINDING_COUNTERCLOCKWISE);

    free(poly);
    free(spath);

    return winding;
error:
    if (spath) {
        free(spath);
    }
    return SHAPE_WINDING_AMBIGUOUS;
}

int prepare_shapes_cb(BSTreeNode *node, void *ctx) {
    PotentialShape *pshape = node->data;
    ShapeMatcher *matcher = ctx;

    if (pshape->path) {
        VPath_destroy(pshape->path);
    }

    VPoint *start = DArray_get(matcher->points, 0);

    pshape->path = Shape_get_path(pshape->shape, *start,
            matcher->initial_segment_length,
            matcher->initial_segment_angle,
            matcher->intended_convex_winding,
            &pshape->farthest);

    return 1;
}

void ShapeMatcher_prepare_to_match(ShapeMatcher *matcher) {
    BSTree_traverse(matcher->potential_shapes, prepare_shapes_cb, matcher);
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
        matcher->intended_convex_winding =
            ShapeMatcher_calc_winding(matcher);
        ShapeMatcher_prepare_to_match(matcher);
    }

    return 1;
error:
    return 0;
}

int widdle_cb(BSTreeNode *node, void *ctx) {
    PotentialShape *pshape = node->data;
    ShapeMatcher *matcher = ctx;

    int matched_count = DArray_count(pshape->matched_points);
    if (pshape->path->num_points > matched_count) {
        VPoint *last_point = DArray_last(matcher->points);
        VPoint next_point = pshape->path->points[matched_count];
        float catch_dist = matcher->vertex_catch_tolerance *
            matcher->initial_segment_length;
        float point_dist = VPoint_distance(*last_point, next_point);

        if (point_dist <= catch_dist) {
            MatchedPoint *mpoint = malloc(sizeof(MatchedPoint));
            mpoint->index = DArray_count(matcher->points) - 1;
            mpoint->distance = point_dist;

            DArray_push(pshape->matched_points, mpoint);

            if (matched_count + 1 == pshape->path->num_points) {
                matcher->matched_shape = pshape;
            }
        } else {
            // No? How sloppy is the user?
            int seg_idx = matched_count - 2;
            ShapeSegment segment = pshape->shape->segments[seg_idx];
            float ideal_length =
                segment.length * matcher->initial_segment_length;
            float max_length =
                ideal_length * matcher->slop_tolerance;

            // Calculate the length starting from the last matching point,
            // through the rest of our points, to the target point.
            float run_length = 0;
            MatchedPoint *last_matched = DArray_last(pshape->matched_points);
            int pt_idx = last_matched->index;
            int i = 0;
            VPoint *last_pt_checked = DArray_get(matcher->points, pt_idx);
            for (i = pt_idx + 1; i < DArray_count(matcher->points); i++) {
                VPoint *run_point = DArray_get(matcher->points, i);
                run_length += VPoint_distance(*last_pt_checked, *run_point);
                last_pt_checked = run_point;
            }
            run_length += VPoint_distance(*last_pt_checked, next_point);

            if (run_length > max_length) {
                // Too sloppy, throw out the shape!
                log_info("Shape matcher: Too sloppy for %s",
                    pshape->shape->name);

                if (!matcher->marked_shape_keys) {
                    matcher->marked_shape_keys =
                        DArray_create(sizeof(void *),
                                      matcher->potential_shapes->count);
                }
                DArray_push(matcher->marked_shape_keys, pshape->shape->name);
            }
        }
    }

    return 1;
}

void ShapeMatcher_clear_marked_shapes(ShapeMatcher *matcher) {
    if (matcher->marked_shape_keys) {
        int i = 0;
        for (i = 0; i < DArray_count(matcher->marked_shape_keys); i++) {
            void *key = DArray_get(matcher->marked_shape_keys, i);
            BSTree_delete(matcher->potential_shapes, key);
        }

        DArray_destroy(matcher->marked_shape_keys);
        matcher->marked_shape_keys = NULL;
    }
}

int ShapeMatcher_commit_point(ShapeMatcher *matcher) {
    check(matcher != NULL, "No matcher to commit");

    DArray_push(matcher->points, matcher->staged_point);
    matcher->staged_point = NULL;

    if (matcher->intended_convex_winding == SHAPE_WINDING_AMBIGUOUS) return 1;

    BSTree_traverse(matcher->potential_shapes, widdle_cb, matcher);

    ShapeMatcher_clear_marked_shapes(matcher);

    return 1;
error:
    return 0;
}

int ShapeMatcher_end(ShapeMatcher *matcher) {
    check(matcher != NULL, "No matcher to end");
    if (matcher->state == SHAPE_MATCHER_STATE_ENDED) return 0;

    assert(matcher->state == SHAPE_MATCHER_STATE_RUNNING);

    if (matcher->matched_shape) {
        float catch_dist = matcher->vertex_catch_tolerance *
            matcher->initial_segment_length;
        float dist_sum = 0;
        int i = 0;
        int matched_count = DArray_count(matcher->matched_shape->matched_points);
        for (i = 0; i < matched_count; i++) {
            MatchedPoint *matched =
                DArray_get(matcher->matched_shape->matched_points, i);
            dist_sum += matched->distance;
        }

        float max_dist = catch_dist * (matched_count - 2);
        float accuracy = 1.f - dist_sum / max_dist;
        log_info("Shape Matcher: Matched %s (%.02f%%)",
                matcher->matched_shape->shape->name,
                accuracy);

        // Call a hook with shape and accuracy
    } else {
        // Failed or cancelled. Call a different hook?
    }

    return 1;
error:
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

typedef struct ShapePathCtx {
    ShapeMatcher *matcher;
    VPath **paths;
    int num_paths;
    int i;
} ShapePathCtx;

int pshape_path_ambiguous_cb(BSTreeNode *node, void *context) {
    ShapePathCtx *ctx = context;
    PotentialShape *pshape = node->data;

    VPoint *start = DArray_get(ctx->matcher->points, 0);

    // CCW
    ctx->paths[ctx->i] = Shape_get_path(pshape->shape, *start,
        ctx->matcher->initial_segment_length,
        ctx->matcher->initial_segment_angle,
        SHAPE_WINDING_COUNTERCLOCKWISE, NULL);
    ctx->i++;

    // CW
    ctx->paths[ctx->i] = Shape_get_path(pshape->shape, *start,
        ctx->matcher->initial_segment_length,
        ctx->matcher->initial_segment_angle,
        SHAPE_WINDING_CLOCKWISE, NULL);
    ctx->i++;

    return 1;
}

int pshape_path_intended_cb(BSTreeNode *node, void *context) {
    ShapePathCtx *ctx = context;
    PotentialShape *pshape = node->data;

    ctx->paths[ctx->i] = VPath_create(pshape->path->points,
                                      pshape->path->num_points);
    ctx->i++;

    return 1;
}

void ShapeMatcher_get_potential_shape_paths(ShapeMatcher *matcher,
        VPath ***paths, int *num_paths) {
    check(paths != NULL, "paths can't be NULL");
    check(num_paths != NULL, "num_paths can't be NULL");
    if (matcher->initial_segment_length == 0) {
        *paths = NULL;
        *num_paths = 0;
        return;
    }

    ShapePathCtx ctx = { .matcher = matcher, .i = 0 };
    if (matcher->intended_convex_winding == SHAPE_WINDING_AMBIGUOUS) {
        ctx.num_paths = matcher->potential_shapes->count * 2;
        ctx.paths = malloc(ctx.num_paths * sizeof(VPath *));

        BSTree_traverse(matcher->potential_shapes, pshape_path_ambiguous_cb,
                        &ctx);
    } else {
        ctx.num_paths = matcher->potential_shapes->count;
        ctx.paths = malloc(ctx.num_paths * sizeof(VPath *));

        BSTree_traverse(matcher->potential_shapes, pshape_path_intended_cb,
                        &ctx);
    }

    *paths = ctx.paths;
    *num_paths = ctx.num_paths;

    return;
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////

typedef struct ConnectDotsCtx {
    ShapeMatcher *matcher;
    VCircle *circles;
    int num_circles;
    int i;
} ConnectDotsCtx;

int pshape_dot_ambiguous_cb(BSTreeNode *node, void *context) {
    ConnectDotsCtx *ctx = context;
    PotentialShape *pshape = node->data;

    VPoint *start = DArray_get(ctx->matcher->points, 0);

    float radius = ctx->matcher->vertex_catch_tolerance *
        ctx->matcher->initial_segment_length;

    // CCW
    VPath *ccw_path = Shape_get_path(pshape->shape, *start,
        ctx->matcher->initial_segment_length,
        ctx->matcher->initial_segment_angle,
        SHAPE_WINDING_COUNTERCLOCKWISE, NULL);
    if (ccw_path->num_points > 2) {
        VCircle circle = {
            .radius = radius,
            .center = ccw_path->points[2]
        };
        ctx->circles[ctx->i] = circle;
        ctx->i++;
    }
    VPath_destroy(ccw_path);

    // CW
    VPath *cw_path = Shape_get_path(pshape->shape, *start,
        ctx->matcher->initial_segment_length,
        ctx->matcher->initial_segment_angle,
        SHAPE_WINDING_CLOCKWISE, NULL);
    if (cw_path->num_points > 2) {
        VCircle circle = {
            .radius = radius,
            .center = cw_path->points[2]
        };
        ctx->circles[ctx->i] = circle;
        ctx->i++;
    }
    VPath_destroy(cw_path);

    return 1;
}

int pshape_dot_intended_cb(BSTreeNode *node, void *context) {
    ConnectDotsCtx *ctx = context;
    PotentialShape *pshape = node->data;

    VPath *path = pshape->path;
    float radius = ctx->matcher->vertex_catch_tolerance *
        ctx->matcher->initial_segment_length;

    int match_count = DArray_count(pshape->matched_points);
    if (path->num_points > match_count) {
        VCircle circle = {
            .radius = radius,
            .center = path->points[match_count]
        };
        ctx->circles[ctx->i] = circle;
        ctx->i++;
    }

    return 1;
}
void ShapeMatcher_get_connect_dots(ShapeMatcher *matcher, VCircle **circles,
        int *num_circles) {
    check(circles != NULL, "circles can't be NULL");
    check(num_circles != NULL, "num_circles can't be NULL");
    if (matcher->initial_segment_length == 0) {
        *circles = NULL;
        *num_circles = 0;
        return;
    }

    ConnectDotsCtx ctx = { .matcher = matcher, .i = 0 };
    if (matcher->intended_convex_winding == SHAPE_WINDING_AMBIGUOUS) {
        ctx.num_circles = matcher->potential_shapes->count * 2;
        ctx.circles = malloc(ctx.num_circles * sizeof(VCircle));

        BSTree_traverse(matcher->potential_shapes, pshape_dot_ambiguous_cb,
                        &ctx);
    } else {
        ctx.num_circles = matcher->potential_shapes->count;
        ctx.circles = malloc(ctx.num_circles * sizeof(VCircle));

        BSTree_traverse(matcher->potential_shapes, pshape_dot_intended_cb,
                        &ctx);
    }

    *circles = ctx.circles;
    *num_circles = ctx.num_circles;

    return;
error:
    return;
}
