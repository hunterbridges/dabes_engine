#ifndef __vpolygon_h
#define __vpolygon_h
#include "vpoint.h"
#include "vrect.h"

typedef struct VPolygon {
    VPoint origin;
    int num_points;
    VPoint points[];
} VPolygon;

VPolygon *VPolygon_create(int num_points, VPoint *points);
VPolygon *VPolygon_from_rect(VRect rect);
void VPolygon_destroy(VPolygon *poly);
void VPolygon_set_point(VPolygon *poly, int i, VPoint point);
VPoint VPolygon_get_point(VPolygon *poly, int i);
void VPolygon_wind(VPolygon *poly, short int clockwise);
int VPolygon_is_clockwise(VPolygon *poly);

#endif
