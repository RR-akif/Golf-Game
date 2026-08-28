#ifndef TYPES_H //In order to prevent redundancy.Such as physics.h also includes this "types.h" , and i am including both types.h and physics.h inside ball.h, so all of the data will be defined twice.
#define TYPES_H // If "types.h" is not defined then at first then define types.h, and if it is already defined, then don't open this file

#include "raylib.h"

//SURFACE TYPES
typedef enum
{
    surf_green,
    surf_fairway,
    surf_sand,
    surf_ice,
    surf_mud,
    surf_water,
    surf_boost,
    surf_count,
}SurfaceType;


//Ballstate-condition of the ball
typedef enum
{
    ball_aim,
    ball_charge,
    ball_roll,
    ball_sunk,
    ball_oob, 
}BallState;
//OOB - Out of bounds


//Ball's data
typedef struct 
{
    Vector2 pos;
    Vector2 vel;
    float radius;
    BallState state;
    int strokes;
    Vector2 lastsafepos; //When the ball is in out of bounds state, then it is kept to its final safe position
}Ball;


//Small building block structures
typedef struct
{
    Rectangle rect; // width,height,x and y coordinate of the rect's top left corner
    float restitution; // How does the ball react after the collision "restitution"
}Wall;


//for zone
typedef struct 
{
    Rectangle area; // Built in datatype- area= specifies the region of the current zone
    SurfaceType type; // User defined datatype - that specifies the type of the surface whether there is mud or water of green
    Vector2 wind; // Built in datatype- finds the condition of wind in the current zone
}Zone;


//Entire golf info
typedef struct
{
    Wall walls[32]; //max size can be 32
    int wallcount; // actual number of walls
    
    Zone zones[16];
    int zonecount;

    Vector2 cupPos; //position of the hole or target
    float cupradius; // When the ball is around the cup(target) , then how much radius will be considered as a successful shot

    Rectangle bounds; //Whole acceptable region of the rectangular field
    Rectangle dropzone; //If the ball is in outb of bounds state, then move the ball to the final saest position
}Hole;


#endif
