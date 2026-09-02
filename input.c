#include "input.h"
#include <raylib.h>
#include <raymath.h>
#include <math.h>




Vector2 DirFromAngle(float angle){
    return (Vector2){cosf(angle),sinf(angle)};
}

void InputInit(InputSystem *sys){
    sys ->aim_angle = 0.0f;
    sys -> right_held_time = 0.0f;
    sys -> right_drag_distance = 0.0f;
    sys -> rightwasdown = false;
}







