#include "vpolygon.h"

VPolygon *VPolygon_create(int num_points, VPoint *points) {
    VPolygon *poly = calloc(1, sizeof(VPolygon) + num_points * sizeof(VPoint));
    check(poly != NULL, "Couldn't create VPolygon");

    poly->num_points = num_points;
    if (points) {
        memcpy(poly->points, points, sizeof(VPoint) * num_points);
    }

    return poly;
error:
    return NULL;
}

VPolygon *VPolygon_from_rect(VRect rect) {
    VPolygon *poly = VPolygon_create(4, NULL);
    check(poly != NULL, "Couldn't create VPolygon for rect");

    VPolygon_set_point(poly, 0, rect.tl);
    VPolygon_set_point(poly, 1, rect.tr);
    VPolygon_set_point(poly, 2, rect.br);
    VPolygon_set_point(poly, 3, rect.bl);

    return poly;
error:
    return NULL;
}

void VPolygon_destroy(VPolygon *poly) {
    check(poly != NULL, "No VPolygon to destroy");
    free(poly);
    return;
error:
    return;
}

void VPolygon_set_point(VPolygon *poly, int i, VPoint point) {
    check(poly != NULL, "No VPolygon to set point in");
    check(i >= 0 && i < poly->num_points, "Index out of bounds for VPolygon");
    poly->points[i] = point;
    return;
error:
    return;
}

VPoint VPolygon_get_point(VPolygon *poly, int i) {
    check(poly != NULL, "No VPolygon to get point from");
    check(i >= 0 && i < poly->num_points, "Index out of bounds for VPolygon");
    return poly->points[i];
error:
    return VPointZero;
}

void VPolygon_reverse(VPolygon *poly) {
    VPoint old[poly->num_points];
    memcpy(old, poly->points, sizeof(VPoint) * poly->num_points);
    int i = 0;
    for (i = 0; i < poly->num_points; i++) {
        int n = poly->num_points - i - 1;
        poly->points[i] = old[n];
    }
}

void VPolygon_wind(VPolygon *poly, short int clockwise) {
    check(poly != NULL, "No VPolygon to wind");
    if (clockwise == VPolygon_is_clockwise(poly)) return;
    VPolygon_reverse(poly);
    return;
error:
    return;
}

int VPolygon_is_clockwise(VPolygon *poly) {
    check(poly != NULL, "No VPolygon to check winding");
    int i = 0;
    float sum = 0;
    for (i = 0; i < poly->num_points; i++) {
        VPoint p1 = VPolygon_get_point(poly, i);
        int next_i = (i + 1) % poly->num_points;
        VPoint p2 = VPolygon_get_point(poly, next_i);
        float edge = p1.x * p2.y - p2.x * p1.y;
        sum += edge;
    }

    return sum > 0;
error:
    return 0;
}
