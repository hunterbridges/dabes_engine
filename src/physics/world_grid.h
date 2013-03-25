#ifndef __physics_world_grid_h
#define __physics_world_grid_h
#include "physics.h"
#include "fixture.h"

typedef enum {
    WORLDGRIDMEMBER_FIXTURE,
    WORLDGRIDMEMBER_TILE
} WorldGridMemberType;

typedef struct {
    WorldGridMemberType member_type;
    union {
        Fixture *fixture;
        void *raw;
    };
} WorldGridMember;

int WorldGridMember_is_equal(WorldGridMember *a, WorldGridMember *b);

typedef struct WorldGridPoint {
    WorldGridMember owner;
    PhysPoint point;
} WorldGridPoint;

WorldGridPoint *WorldGridPoint_create(PhysPoint point, WorldGridMember owner);
void WorldGridPoint_destroy(WorldGridPoint *wgpoint);
int WorldGridPoint_is(WorldGridPoint *wgpoint, PhysPoint point,
        WorldGridMember owner);

typedef struct WorldGridCell {
    int row;
    int col;
    List *points;
} WorldGridCell;

WorldGridCell *WorldGridCell_create(int row, int col);
void WorldGridCell_destroy(WorldGridCell *cell);

typedef struct WorldGrid {
    int rows;
    int cols;
    double grid_size;
    DArray *cells;
} WorldGrid;

WorldGrid *WorldGrid_create(int rows, int cols, double grid_size);
void WorldGrid_destroy(WorldGrid *grid);

int WorldGrid_update_fixture(WorldGrid *grid, Fixture *fixture);

int WorldGrid_add_fixture(WorldGrid *grid, Fixture *fixture);
int WorldGrid_add_box(WorldGrid *grid, PhysBox box, WorldGridMember owner);
int WorldGrid_add_point(WorldGrid *grid, PhysPoint point,
        WorldGridMember owner);

int WorldGrid_remove_fixture(WorldGrid *grid, Fixture *fixture);
int WorldGrid_remove_box(WorldGrid *grid, PhysBox box, WorldGridMember owner);
int WorldGrid_remove_point(WorldGrid *grid, PhysPoint point,
        WorldGridMember owner);

WorldGridCell *WorldGrid_cell_for_point(WorldGrid *grid, PhysPoint point);
List *WorldGrid_cells_for_box(WorldGrid *grid, PhysBox box);
List *WorldGrid_members_near_fixture(WorldGrid *grid, Fixture *fixture);

#endif
