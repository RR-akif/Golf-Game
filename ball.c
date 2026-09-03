#include "ball.h"
#include "physics.h"
#include "raymath.h"
#include <math.h>


//Define
#define MAX_LAUNCH_SPEED 900.00
#define REST_SPEED 12.00
#define MAX_STEP_DISTANCE 4.00
#define MAX_SUBSTEPS 8
#define WALL_TANGENTIAL 0.96

void Ballinit(Ball *ball,Vector2 teepos)
{
    ball->pos=teepos;
    ball->vel=Vector2Zero(); //Initializing the starting velocity as zero 
    ball->radius=7.00;
    ball->strokes=0;
    ball->lastsafepos=teepos;
}

float Ballspeed(Ball *ball)
{
    return Vector2Length(ball->vel);
}

bool BallAtRest(Ball *ball)
{
    return ball->state==BALL_AIM;
}

void Balllaunch(Ball *ball,Vector2 dir,float power)
{
    
}