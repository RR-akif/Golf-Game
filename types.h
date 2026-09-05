#ifndef TYPES_H //In order to prevent redundancy.Such as physics.h also includes this "types.h" , and i am including both types.h and physics.h inside ball.h, so all of the data will be defined twice.
#define TYPES_H // If "types.h" is not defined then at first then define types.h, and if it is already defined, then don't open this file

#include "raylib.h"

//Define
#define MAX_WALLS 128
#define MAX_ZONES 32
#define MAX_HOLES 18
#define MAX_OBSTACLES 32

//SURFACE TYPES
typedef enum
{
    SURF_GREEN,
    SURF_FAIRWAY,
    SURF_SAND,
    SURF_ICE,
    SURF_MUD,
    SURF_WATER,
    SURF_BOOST,
    SURF_COUNT, /* it will automatically update the suraface numbers that i have created.....(rafin) 
                    so next time we can just add a new surface, without changing the count*/
}SurfaceType;


//Ballstate-condition of the ball
typedef enum
{
    BALL_AIM,
    BALL_CHARGE,
    BALL_ROLL,
    BALL_SUNK,
    BALL_OOB, 
}BallState;
//OOB - Out of bounds

//Defining current state of the game
typedef enum
{
    GS_MENU, // shows menu or options
    GS_PLAYING, //player is at playing mode
    GS_HOLE_DONE, //shows hole completed informations
    GS_SCOREBOARD, //shows scores or results
    GS_PAUSED // stops gameplay while showing pause menu
}GameStateId;


//Ball's data
typedef struct 
{
    Vector2 pos;
    Vector2 vel;
    float radius;
    BallState state;
    int strokes;
    Vector2 last_safe_pos; //When the ball is in out of bounds state, then it is kept to its final safe position
}Ball;


//Small building block structures
typedef struct
{
    Rectangle rect; // width,height,x and y coordinate of the rect's top left corner     typedef struct{float x,float y,float width,float height}Rectangle
    float bounce; // How does the ball react after the collision "restitution"
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
    Wall walls[MAX_WALLS]; //max size of array
    int wall_count; // actual number of walls
    
    Zone zones[MAX_ZONES];
    int zone_count;

    int number; //which number of hole this is
    int par; //expected number of strokes to finish the hole
    Vector2 tee_pos; //Where the ball  starts
    Vector2 cup_pos; //position of the hole or target
    float cup_radius; // When the ball is around the cup(target) , then how much radius will be considered as a successful shot

    Rectangle bounds; //Whole acceptable region of the rectangular field(boundarries), course is named as bounds
    Rectangle drop_zone; //If the ball is in out of bounds state, then move the ball to the final safest position. This is a special rectangle where the ball is put only when tha ball goes out of bounds state
}Hole;


typedef struct 
{
    GameStateId state;
    Camera2D camera;              //typedef struct{Vector2 target;Vector2 offset;float rotation;float zoom}Camera2D;
    Ball ball;
    Hole hole;
    int hole_index;  //there can be multuple holes per level, so it defines the indices of each hole
    int scores[MAX_HOLES];  //we will calculate scores for individual holes, such as scores[0]=4 that would mean total score for hole 1 is 4.Hence, to get final score we have to accumulate scores for all holes.
}Game;


#endif
