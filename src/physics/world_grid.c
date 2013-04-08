#include "world_grid.h"

int WorldGridMember_is_equal(WorldGridMember *a, WorldGridMember *b) {
    if (a->member_type != b->member_type) return 0;
    switch (a->member_type) {
        case WORLDGRIDMEMBER_FIXTURE:
            if (a->fixture != b->fixture) return 0;
            break;

        default:
            if (a->raw != b->raw) return 0;
    }

    return 1;
}

WorldGridPoint *WorldGridPoint_create(VPoint point, WorldGridMember owner) {
    WorldGridPoint *wgpoint = malloc(sizeof(WorldGridPoint));
    check(wgpoint != NULL, "Couldn't make world grid point");
    wgpoint->point = point;
    wgpoint->owner = owner;

    return wgpoint;
error:
    return NULL;
}

void WorldGridPoint_destroy(WorldGridPoint *wgpoint) {
    check(wgpoint != NULL, "No wgpoint to destroy");
    free(wgpoint);
error:
    return;
}

int WorldGridPoint_is(WorldGridPoint *wgpoint, VPoint point,
        WorldGridMember owner) {
    if (wgpoint->point.x != point.x) return 0;
    if (wgpoint->point.y != point.y) return 0;
    if (!WorldGridMember_is_equal(&wgpoint->owner, &owner)) return 0;
    return 1;
}

WorldGridCell *WorldGridCell_create(int row, int col) {
    WorldGridCell *cell = malloc(sizeof(WorldGridCell));
    check(cell != NULL, "Couldn't create world grid cell");

    cell->row = row;
    cell->col = col;
    cell->points = List_create();

    return cell;
error:
    return NULL;
}

void WorldGridCell_destroy(WorldGridCell *cell) {
    check(cell != NULL, "No cell to destroy");
    List_clear_destroy(cell->points);
    free(cell);
    return;
error:
    return;
}

WorldGrid *WorldGrid_create(int rows, int cols, double grid_size) {
    int cells = rows * cols;
    WorldGrid *grid = malloc(sizeof(WorldGrid));
    check(grid != NULL, "Couldn't create grid");

    grid->rows = rows;
    grid->cols = cols;
    grid->grid_size = grid_size;
    grid->cells = DArray_create(sizeof(WorldGridCell), cells);

    return grid;
error:
    return NULL;
}

void WorldGrid_destroy(WorldGrid *grid) {
    check(grid != NULL, "No grid to destroy");
    int cells = grid->rows * grid->cols;
    int i = 0;
    for (i = 0; i < cells; i++) {
        WorldGridCell *cell = DArray_get(grid->cells, i);
        if (cell == NULL) continue;
        WorldGridCell_destroy(cell);
    }
    DArray_destroy(grid->cells);
    free(grid);
    return;
error:
    return;
}

int WorldGrid_update_fixture(WorldGrid *grid, Fixture *fixture) {
    check(grid != NULL, "No grid to update.");
    VRect old = fixture->history[1];
    VRect new = fixture->history[0];
    WorldGridMember owner = {
        .member_type = WORLDGRIDMEMBER_FIXTURE
    };
    owner.fixture = fixture;
    WorldGrid_remove_box(grid, old, owner);
    WorldGrid_add_box(grid, new, owner);
    return 1;
error:
    return 0;
}

int WorldGrid_add_fixture(WorldGrid *grid, Fixture *fixture) {
    check(grid != NULL, "No grid to add to.");
    VRect box = Fixture_real_box(fixture);
    WorldGridMember owner = {
        .member_type = WORLDGRIDMEMBER_FIXTURE
    };
    owner.fixture = fixture;
    int rc = WorldGrid_add_box(grid, box, owner);
    return rc;
error:
    return 0;
}

int WorldGrid_add_box(WorldGrid *grid, VRect box, WorldGridMember owner) {
    check(grid != NULL, "No grid to add to.");
    unsigned int i = 0;

    int rc = 1;
    for (i = 0; i < 4; i++) {
        VPoint point = VRect_vertex(box, i);
        int this_rc = WorldGrid_add_point(grid, point, owner);
        rc = rc && this_rc;
    }

    return rc;
error:
    return 0;
}

int WorldGrid_add_point(WorldGrid *grid, VPoint point,
        WorldGridMember owner) {
    check(grid != NULL, "No grid to add to.");
    int row = point.y / grid->grid_size;
    int col = point.x / grid->grid_size;
    if (col < 0) return 0;
    if (col >= grid->cols) return 0;
    if (row < 0) return 0;
    if (row >= grid->rows) return 0;

    int idx = row * grid->cols + col;
    WorldGridCell *cell = DArray_get(grid->cells, idx);
    if (cell == NULL) {
        cell = WorldGridCell_create(row, col);
        DArray_set(grid->cells, idx, cell);
    }

    if (cell) {
      WorldGridPoint *wgpoint = WorldGridPoint_create(point, owner);
      List_push(cell->points, wgpoint);
    }

    return 1;
error:
    return 0;
}

int WorldGrid_remove_fixture(WorldGrid *grid, Fixture *fixture) {
    check(grid != NULL, "No grid to remove from.");
    VRect box = Fixture_real_box(fixture);
    WorldGridMember owner = {
        .member_type = WORLDGRIDMEMBER_FIXTURE
    };
    owner.fixture = fixture;
    int rc = WorldGrid_remove_box(grid, box, owner);
    return rc;
error:
    return 0;
}

int WorldGrid_remove_box(WorldGrid *grid, VRect box, WorldGridMember owner) {
    check(grid != NULL, "No grid to remove from.");
    int i = 0;

    int rc = 1;
    for (i = 0; i < 4; i++) {
        VPoint point = VRect_vertex(box, i);
        int this_rc = WorldGrid_remove_point(grid, point, owner);
        rc = rc && this_rc;
    }

    return rc;
error:
    return 0;
}

int WorldGrid_remove_point(WorldGrid *grid, VPoint point,
        WorldGridMember owner) {
    check(grid != NULL, "No grid to remove from.");

    WorldGridCell *cell = WorldGrid_cell_for_point(grid, point);
    if (cell == NULL) return 0;

    check(cell->points != NULL, "No points found in cell %p <%d, %d>",
            cell, cell->col, cell->row);
    ListNode *found = NULL;
    LIST_FOREACH(cell->points, first, next, current) {
        WorldGridPoint *wgpoint = current->value;
        if (WorldGridPoint_is(wgpoint, point, owner)) {
            found = current;
            break;
        }
    }

    if (found) {
        WorldGridPoint *wgpoint = List_remove(cell->points, found);
        free(wgpoint);
        return 1;
    }

    debug("Couldn't remove point <%f, %f>", point.x, point.y);
    if (owner.fixture == NULL) {
        debug("oh boy");
    }

    return 0;
error:
    return 0;
}

WorldGridCell *WorldGrid_cell_for_point(WorldGrid *grid, VPoint point) {
    check(grid != NULL, "No grid to search.");
    int row = point.y / grid->grid_size;
    int col = point.x / grid->grid_size;
    if (col < 0) return NULL;
    if (col >= grid->cols) return NULL;
    if (row < 0) return NULL;
    if (row >= grid->rows) return NULL;


    int idx = row * grid->cols + col;
    return DArray_get(grid->cells, idx);
error:
    return NULL;
}

List *WorldGrid_cells_for_box(WorldGrid *grid, VRect box) {
    check(grid != NULL, "No grid to search.");

    List *cells = NULL;

    int i = 0;
    for (i = 0; i < 4; i++) {
        VPoint point = VRect_vertex(box, i);
        WorldGridCell *cell = WorldGrid_cell_for_point(grid, point);
        if (cell == NULL) continue;
        if (cells == NULL) {
          cells = List_create();
          check(cells != NULL, "Couldn't create cells list");
        }

        if (!List_contains(cells, cell, NULL)) List_push(cells, cell);
    }

    return cells;
error:
    return NULL;
}

List *WorldGrid_members_near_fixture(WorldGrid *grid, Fixture *fixture) {
    List *cells = NULL;
    check(grid != NULL, "No grid to search.");
    VRect box = Fixture_real_box(fixture);
    cells = WorldGrid_cells_for_box(grid, box);
    if (cells == NULL) return NULL;

    List *members = List_create();
    check(members != NULL, "Couldn't create members list.");

    LIST_FOREACH(cells, first, next, current) {
        WorldGridCell *cell = current->value;

        ListNode *point_node = NULL;
        for (point_node = cell->points->first; point_node != NULL;
                point_node = point_node->next) {
            WorldGridPoint *point = point_node->value;
            WorldGridMember *member = &(point->owner);

            if (member->fixture != fixture && !List_contains(members, member,
                        (List_equal_cb)WorldGridMember_is_equal)) {
                List_push(members, member);
            }
        }
    }

    List_destroy(cells);

    return members;
error:
    if (cells) List_destroy(cells);
    return NULL;
}

VRect WorldGrid_box_for_cell(WorldGrid *grid, int col, int row) {
  VRect box = {
    .tl = {
      grid->grid_size * col,
      grid->grid_size * row
    },
    .tr = {
      grid->grid_size * col + grid->grid_size,
      grid->grid_size * row
    },
    .br = {
      grid->grid_size * col + grid->grid_size,
      grid->grid_size * row + grid->grid_size
    },
    .bl = {
      grid->grid_size * col,
      grid->grid_size * row + grid->grid_size
    }
  };
  return box;
}

