#ifndef PHYSICS_H
#define PHYSICS_H

#include "types.h"

float Frictionof(SurfaceType s);
SurfaceType Surfaceat(const Hole *hole,Vector2 p);
Vector2 Windat(const Hole *hole, Vector2 p);

//Apply forces
void ApplyFriction(Ball *ball,float decel,float dt);
void Applywind(Ball *b,Vector2 wind,float dt);

//Check collisions and bouncing
void CheckWallCollision(Ball *b,Hole *h);
int CheckCupCollision(Ball *b,Hole *h);

#endif