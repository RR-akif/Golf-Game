#include "physics.h"
#include "raymath.h"
#include <math.h>

//Frictions for distinct surfaces, declaring an array, as the values cannot be modified so we should use const 
const float kfriction[surf_count]={
    [surf_green]=420.0,
    [surf_fairway]=300.f,
    [surf_sand]=1400.00,
    [surf_ice]=90.0,
    [surf_mud]=2600.00,
    [surf_water]=420.0,
    [surf_boost]=-600.0, //boosts the velocity
};

float Frictionof(SurfaceType s)
{
    if(s<0 || s>=surf_count) return kfriction[surf_green]; // To avoid array out of bounds
    return kfriction[s];
}

//Determining the surface at which the ball is lying currently
SurfaceType Surfaceat(const Hole *hole,Vector2 p) // hole values should be fixed, so we used const
{
    SurfaceType found=surf_green;

    for(int i=0;i<hole->zonecount;i++)
    {
        if(CheckCollisionPointRec(p,hole->zones[i].area)) // checks whether the point p falls anywhere inside (or on the edge of)that rectangle
        {
            found=hole->zones[i].type; //storing the type of the surface
        }
    }
    return found;
}


//Adding wind at some specific zones
Vector2 Windat(const Hole *hole, Vector2 p)
{
    Vector2 w={0.0 , 0.0};
    for(int i=0;i<hole->zonecount;i++)
    {
        if(CheckCollisionPointRec(p,hole->zones[i].area))
        {
            w=Vector2Add(w,hole->zones[i].wind); //Accumulate the values of wind for distinct surfaces, wind is an acceleration not a velocity
        }
    }
    return w;
}


// Add friction
void ApplyFriction(Ball *ball,float decel,float dt)
{
    float speed=Vector2Length(ball->vel);
    if(speed<=0.00) return ;

    float newspeed=speed - decel*dt;
    if(newspeed<=0.00) newspeed=0.0; //when newspeed becomes negative and if we dont convert it to zero, then both x and y component of ball->vel will be negative. Then after calculating length, the speed will again be positive.
    if(newspeed>max_ball_speed) newspeed=max_ball_speed; //When ball->vel is boosted due to a certain zone.

    ball->vel=Vector2Scale(ball->vel,newspeed/speed); // The ratio is always less than 1, so the valocity will be reduced gradually.
}

//Apply wind
void Applywind(Ball *b,Vector2 wind,float dt)
{
    b->vel= Vector2Add(b->vel , Vector2Scale(wind,dt)); //wind is an acceleration, so wind*dt makes it a velocity, then sumps up with the previous velocity
}


//check collision with walls and bouncing off
void CheckWallCollision(Ball *b,Hole *h)
{
    for(int i=0;i<h->wallcount;i++)
    {
        if(b->pos.x - b->radius < h->walls[i].rect.x) // left edge of the wall
        {
            b->pos.x = b->radius + h->walls[i].rect.x;
            b->vel.x= -b->vel.x * h->walls[i].restitution;
        }

        if(b->pos.x + b->radius > h->walls[i].rect.x + h->walls[i].rect.width) // Right edge of the wall
        {
            b->pos.x = -b->radius + h->walls[i].rect.x + h->walls[i].rect.width;
            b->vel.x= -b->vel.x * h->walls[i].restitution;
        }

        if(b->pos.y - b->radius < h->walls[i].rect.y) // top edge of the wall
        {
            b->pos.y= b->radius + h->walls[i].rect.y;
            b->vel.y= -b->vel.y * h->walls[i].restitution;
        }

        if(b->pos.y + b->radius < h->walls[i].rect.y + h->walls[i].rect.height) // bottom edge of the wall
        {
            b->pos.y = -b->radius + h->walls[i].rect.y + h->walls[i].rect.height;
            b->vel.y= -b->vel.y * h->walls[i].restitution;
        }
    }
    
    float magnitude= sqrtf(b->vel.x * b->vel.x + b->vel.y * b->vel.y);
    b->vel=(Vector2){b->vel.x,b->vel.y}; // In order to assign a struct we should do typecasting before
    b->vel = Vector2Scale(Vector2Normalize(b->vel), magnitude); // multiplying the magnitude(scalar) by the unit vector of b->vel
}


//Check collision with circular hole
int CheckCupCollision(Ball *b,Hole *h)
{
    float dx= b->pos.x - h->cupPos.x;
    float dy= b->pos.y - h->cupPos.y;
    float distance = sqrtf(dx*dx + dy*dy);
    float capture_dist= h->cupradius - b->radius;

    if(capture_dist < 0.00)
    capture_dist= 0.00;

    if(Vector2Length(b->vel)<=hole_capture_speed && distance<=capture_dist)
    return 1;
    
    return 0;
}


