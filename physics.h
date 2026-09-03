#ifndef PHYSICS_H
#define PHYSICS_H

#include "types.h"

float FrictionOf(SurfaceType s);
SurfaceType SurfaceAt(Hole *hole,Vector2 p);
Vector2 WindAt(Hole *hole, Vector2 p);

//Apply forces
void ApplyFriction(Ball *ball,float decel,float dt);
void ApplyWind(Ball *b,Vector2 wind,float dt);

//Check collisions and bouncing
void CheckWallCollision(Ball *b,Hole *h);
int CheckCupCollision(Ball *b,Hole *h);

#endif