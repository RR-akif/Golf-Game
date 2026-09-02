#ifndef INPUT_H
#define INPUT_H

#include <raylib.h>
#include "types.h"



typedef struct {
    Vector2 aim_direction;
    float aim_angle;
    Vector2 pointer_world;
    bool pointer_valid;

    bool charge_pressed;
    bool charge_held;
    bool charge_released;

    bool cancel;

    bool confirm;
    bool camZoomDelta;

} InputState;

typedef struct{
    float aim_angle;
    float right_held_time;
    float right_drag_distance;
    bool rightwasdown;

} InputSystem;

void InputInit(InputSystem *sys);

InputState InputPoll(InputSystem *sys,Vector2 ballPos,Camera2D cam, float dt);

Vector2 DirFromAngle(float angle);



#endif 
