#ifndef BALL_H
#define BALL_H

#include "types.h"

//All declared functions
void BallInit(Ball *ball,Vector2 tee_pos); //Initializing the position and velocity of the ball
float BallSpeed(Ball *ball); // determine the magnitude of its velocity
bool BallAtRest(Ball *ball); //Checking whether the ball is at rest or not
void BallLaunch(Ball *ball,Vector2 dir,float power); //After effect of using the powermeter
void BallComeToRest(Ball *ball); 
void CheckCup(Ball *ball,Hole *hole); 
void CheckHazards(Ball *ball,Hole *hole); //Checking out of bounds state(water / outside the playable region bounds)
void DropBall(Ball *ball, Hole *hole); //To handle the after effect of out of bounds state
void StepPhysics(Ball *ball,Hole *hole,float dt); // fixing the rolling state of the ball
void BallUpdate(Ball *ball,Hole *hole,float dt); //Merging all states 

#endif
