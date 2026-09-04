#include "input.h"
#include <raylib.h>
#include <raymath.h>
#include <math.h>


#define aim_distance 40.0f

#define right_click_time 0.25f
#define right_click_distance 6.0f
 
#define zoom_per_notch  0.1f



Vector2 DirFromAngle(float angle){
    return (Vector2){cosf(angle),sinf(angle)};
}

void InputInit(InputSystem *sys){
    sys ->aim_angle = 0.0f;
    sys -> right_held_time = 0.0f;
    sys -> right_drag_distance = 0.0f;
    sys -> rightwasdown = false;
}

InputState InputPoll(InputSystem *sys, Vector2 ballPos, Camera2D cam, float dt){
    InputState in = {0};
    
    in.pointer_world = GetScreenToWorld2D(GetMousePosition(), cam);

    // for aiming the ball.
    Vector2 to_cursor = Vector2Subtract(in.pointer_world,ballPos);
    // check if aim is valid even or not
    if(Vector2LengthSqr(to_cursor) > aim_distance * aim_distance){
        sys->aim_angle = atan2f(to_cursor.y,to_cursor.x);
        in.pointer_valid = true;
    }
    else{
        in.pointer_valid = false;
    }
    in.aim_angle = sys->aim_angle;
    in.aim_direction = DirFromAngle(sys->aim_angle);

    in.charge_pressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    in.charge_held = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    in.charge_released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    in.confirm = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if(IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)){
        sys->right_held_time = 0.0f;
        sys->right_drag_distance = 0.0f;
        sys->rightwasdown = true;
    }
    if(sys->rightwasdown && IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
        sys->right_held_time += dt;
        sys->right_drag_distance = Vector2Length(GetMouseDelta());
    }

    if(sys->rightwasdown && IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)){
        in.cancel = (sys->right_held_time < right_click_time && sys->right_drag_distance < right_click_distance);
        sys->rightwasdown = false;
    }
    in.camZoomDelta = GetMouseWheelMove() * zoom_per_notch;


    return in;

}








