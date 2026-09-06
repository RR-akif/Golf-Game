#ifndef PUTTER_H
#define PUTTER_H


#include <raylib.h>
#include <raymath.h>
#include <math.h>
#include <stdio.h>


#include "ball.h"
#include "types.h"
#include "input.h"


typedef enum{
    putter_idle,
    putter_backswing,
    putter_strike,
    putter_follow,
    putter_recover
} PutterPhase;

typedef struct{
    PutterPhase phase;
    float t; // seconds inside the current phase
    float power;
    float launch_power;
    float aim_angle;
    float offset;
    float dir; // +1 while the bar is filling, -1 while it is emptying
    bool fired;
} Putter;

void putter_init(Putter *p);
void putter_update(Putter *p,const InputState *in,Ball *ball,float dt);
bool PutterIsCharging(const Putter *p);
float PutterPower(const Putter *p);
void putter_draw(const Putter *p,const Ball *ball);

#endif
