#include "level.h"
#include "types.h"
#include <raylib.h>
#include <raymath.h>
#include <string.h>
#include <stdio.h>
#include <math.h>




// editor

void EditorInit(Edtior *e){
    e->tool = TOOL_WALL;
    e->zonetype = SURF_SAND;
    e->dragging = false;
    e->grid_size = 10;
    e->selected = -1;
}