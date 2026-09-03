#include "ball.h"
#include "physics.h"
#include "raymath.h"
#include <math.h>


//Define
#define MAX_LAUNCH_SPEED 900.00
#define REST_SPEED 12.00
#define MAX_STEP_DISTANCE 4.00 // The max distance(in pixels) the ball is allowed to travel in a single physics calculation step without causing tunneling or passing through any object
#define MAX_SUBSTEPS 8   // maximum dynamic number of smaller micro-steps the code decides to split thhe current frame into


void BallInit(Ball *ball,Vector2 teepos)
{
    ball->pos=teepos;
    ball->vel=Vector2Zero(); //Initializing the starting velocity as zero 
    ball->radius=7.00;
    ball->strokes=0;
    ball->lastSafePos=teepos;
    ball->state=BALL_AIM;
}

float BallSpeed(Ball *ball)
{
    return Vector2Length(ball->vel);
}

bool BallAtRest(Ball *ball)
{
    return ball->state==BALL_AIM;
}

void BallLaunch(Ball *ball,Vector2 dir,float power) // dir- the 2D direction the player is aiming , as a 2D vector . power- how hard the shot is (from 0 to max 1)
{
    if(ball->state !=BALL_AIM && ball->state !=BALL_CHARGE)
    return;

    if(power<=0.00) return;
    if(power>1.00) power=1.00;
    if(Vector2LengthSqr(dir)<0.0001) return; //Very close to zero vector, so simply we can neglect it

    ball->vel=Vector2Scale(Vector2Normalize(dir),power * MAX_LAUNCH_SPEED); // ball moves along the desired direction
    ball->state=BALL_ROLL;
    ball->strokes+=1;
}

void BallComeToRest(Ball *ball)
{
    ball->vel=Vector2Zero();
    ball->state=BALL_AIM;
    ball->lastSafePos=ball->pos; //lastSafePos is only updated when the ball comes to a natural stop
}

void CheckCup(Ball *ball,Hole *hole)
{
    if(CheckCupCollision(ball,hole))
    {
        ball->pos=hole->cupPos;
        ball->vel=Vector2Zero();
        ball->state=BALL_SUNK;
        return;
    }
}

void CheckHazards(Ball *ball,Hole *hole)
{
    bool inWater=(SurfaceAt(hole,ball->pos)==SURF_WATER); //check whether the ball is on water or not
    bool outside=!CheckCollisionPointRec(ball->pos,hole->bounds); // Chech whether the ball lies on the playable region(boumds) is so, then it returns 1,so output becomes zero. again, if the ball doesn't lie inside the rectangle, then it returns zero, "outside" becomes true.

    if(inWater || outside)
    {
        ball->vel=Vector2Zero();
        ball->state=BALL_OOB;
    }
}

void DropBall(Ball *ball, Hole *hole) //This function will only be called when the ball goes out of bounds state
{
    Vector2 target=ball->lastSafePos; //Just initialize it by lastsafepos

    if(hole->dropZone.width >0.00 && hole->dropZone.height>0.00) // Just to guarantee whether it is positive or not, nothing crucial
    {
        target=(Vector2){hole->dropZone.x+hole->dropZone.width*0.5 , hole->dropZone.y+hole->dropZone.height*0.5}; //When the ball goes oob state, then the ball will be kept at the center of rectangular dropZone

    }
    ball->pos=target;
    ball->lastSafePos=target;
    ball->vel=Vector2Zero();
    ball->strokes +=1; //One penalty stroke for pushing the ball towards out of bounds state
    ball->state=BALL_AIM;
}

void StepPhysics(Ball *ball,Hole *hole,float dt) //Handling the rolling state of the ball
{
    SurfaceType surf=SurfaceAt(hole,ball->pos);

    ApplyWind(ball,WindAt(hole,ball->pos),dt); //The second parameter was the value of wind vector. To get the value, we used the function windat, as it returns the wind vector
    ApplyFriction(ball,FrictionOf(surf),dt);
    
    ball->pos=Vector2Add(ball->pos,Vector2Scale(ball->vel,dt)); //Updating the ball's position

    CheckCup(ball,hole);
    CheckWallCollision(ball,hole);

    if(ball->state != BALL_ROLL) return;
    CheckHazards(ball,hole);
    if(ball->state != BALL_ROLL) return;

    if(Vector2LengthSqr(ball->vel)<REST_SPEED*REST_SPEED)
    BallComeToRest(ball);
}


void BallUpdate(Ball *ball,Hole *hole,float dt)
{
    switch(ball->state)
    {
        case BALL_AIM:
        case BALL_CHARGE:
        case BALL_SUNK:
        break;                   //Nothing to simulate

        case BALL_OOB:
        DropBall(ball,hole);
        break;

        case BALL_ROLL:
        {
            float speed=Vector2Length(ball->vel);
            //Now if a ball is moving at a high speed,then its single frame jump(vel * dt) might be 50 pixels wide, while a thin wall mey be only 10 pixels thick. So it will cause tunneling (passing through the wall). so it will be safe and more accurate if we split the "dt" into more smaller slices.
            
            int steps=(int)(speed*dt/MAX_STEP_DISTANCE) +1; //speed*dt refers to the covered distance by the ball in a single frame, dividing it by max_step_distance we get the total number of steps in a single frame. To make the minimum steps one, we added one at the end.
            if(steps>MAX_SUBSTEPS) steps=MAX_SUBSTEPS;

            float sub=dt/(float)steps; // slicing the delta time into smaller pieces.
            for(int i=0;i<steps;i++)
            {
                StepPhysics(ball,hole,sub);
                if(ball->state !=BALL_ROLL) break;
            }
            break;
        }
    }
}