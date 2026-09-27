#ifndef INPUT_H
#define INPUT_H

#include <raylib.h>
#include "types.h"



typedef struct {
    // one note, this update every frame,..
    Vector2 aim_direction;
    float aim_angle;
    Vector2 pointer_world; // again for the camera function...... no need to understand this. its a extra feature...
    bool pointer_valid; 
    bool charge_pressed; // for mouse button checking
    bool charge_held; // if it is held or not
    bool charge_released;  // finding the time when to release the ball through this boleean...

    bool cancel; // for RMB..
    
    bool confirm;
    bool cam_zoom_delta; // for akif.....

} InputState;

typedef struct{
    // this is for only saving the snapshot of memory, its more like a memory and we update from InputState struct to here...
    float aim_angle;
    float right_held_time;
    float right_drag_distance; // eta demo version e lagto....
    bool right_was_down;

} InputSystem;

void InputInit(InputSystem *sys);

InputState InputPoll(InputSystem *sys,Vector2 ball_pos,Camera2D cam, float dt);

Vector2 DirFromAngle(float angle);  // helper function for converting...



#endif 
