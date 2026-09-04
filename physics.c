#include "physics.h"
#include "raymath.h"
#include <math.h>



//Define
#define MAX_BALL_SPEED 1400.00
#define CUP_CAPTURE_SPEED 250.00


//Frictions for distinct surfaces, declaring an array, as the values cannot be modified so we should use const   int a[6]={[2]=50,[7]=100}; a]2]=50,a[7]=100, other values are set to zero
const float kfriction[SURF_COUNT]={
    [SURF_GREEN]=420.0,
    [SURF_FAIRWAY]=300.f,
    [SURF_SAND]=1400.00,
    [SURF_ICE]=90.0,
    [SURF_MUD]=2600.00,
    [SURF_WATER]=420.0,
    [SURF_BOOST]=-600.0, //boosts the velocity
};

float FrictionOf(SurfaceType s)
{
    if(s<0 || s>=SURF_COUNT) return kfriction[SURF_GREEN]; // To avoid array out of bounds
    return kfriction[s];
}

//Determining the surface at which the ball is lying currently
SurfaceType SurfaceAt(Hole *hole,Vector2 p)
{
    SurfaceType found=SURF_GREEN; // firstly we assume the ball to be found in ground

    for(int i=0;i<hole->zoneCount;i++) //each zone is a rectangular area , with a surface type and a wind vector
    {
        if(CheckCollisionPointRec(p,hole->zones[i].area)) // checks whether the point p falls anywhere inside (or on the edge of)that rectangle
        {
            found=hole->zones[i].type; //storing the type of the surface
        }
    }
    return found;
}


//Adding wind at  zones
Vector2 WindAt(Hole *hole, Vector2 p)
{
    Vector2 w={0.0 , 0.0};
    for(int i=0;i<hole->zoneCount;i++)
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
    if(newspeed>MAX_BALL_SPEED) newspeed=MAX_BALL_SPEED; //When ball->vel is boosted due to a certain zone.

    ball->vel=Vector2Scale(ball->vel,newspeed/speed); // The ratio is always less than 1, so the valocity will be reduced gradually.
}

//Apply wind
void ApplyWind(Ball *b,Vector2 wind,float dt)
{
    b->vel= Vector2Add(b->vel , Vector2Scale(wind,dt)); //wind is an acceleration, so wind*dt makes it a velocity, then sumps up with the previous velocity
}

//Check collision with wall
void CheckWallCollision(Ball *b, Hole *h)
{
    for (int i = 0; i < h->wallCount; i++)
    {
        Rectangle r=h->walls[i].rect;

        float closestX=fmaxf(r.x,fminf(b->pos.x,r.x + r.width));  //Find the X coordinate of the point on the rectangle that is closest to the ball's center.It forces closestX to stay between r.x and r.x + r.width . If ball is horizontally inside the rectangle then it returns the x coordinate of the center of the ball, if the ball is on the left of the left wall it returns the r.x(left edge) and if the ball is on the right side of the right wall, then it returns the x coordiante of the right edge(r.x + r.x + r.width)
        float closestY=fmaxf(r.y, fminf(b->pos.y, r.y + r.height)); //similarly get the closest y, together they represent the closest coordinate on the wall from the ball's center.

        float dx=b->pos.x-closestX;
        float dy=b->pos.y-closestY; //points to a vector from rectangle to ball (vector acts always along final vector - initial vector)

        float distanceSquared=dx*dx + dy*dy;
        if (distanceSquared<=b->radius*b->radius) // whether the distance is less than the radius of the ball
        {
            float distance = sqrtf(distanceSquared);

            Vector2 normal; //The direction perpendicular to the surface the the ball should be pushed away from
            if (distance > 0.0)
            {
                normal = (Vector2){ 
                    dx / distance,
                    dy / distance    // making "normal" a unit vector "unit normal"= it has only direction and length 1, so that at the time of the ball being bounced off, we can just alter the velocity direction along the "unit normal " vector.
                };
            }
            else              // when distance is zero (ball is inside the rectangle) then dx/distance this would lead to 0/0.
            {
                float left=fabsf(b->pos.x - r.x);
                float right=fabsf(b->pos.x - (r.x + r.width));
                float top=fabsf(b->pos.y - r.y);
                float bottom=fabsf(b->pos.y - (r.y + r.height)); //determining distances form each edge of the wall

                float minDist=fminf(fminf(left, right),
                                      fminf(top, bottom)); // Finding the distance of the ball from the nearest edge among four edges.

                if (minDist == left)
                    normal=(Vector2){-1.0, 0.0};  // As distance from the left edge is shortest , so we can assume that the ball entered the wall through the left side, and so it should be bounced off from the left edge.
                else if (minDist == right)
                    normal=(Vector2){1.0, 0.0};
                else if (minDist == top)
                    normal=(Vector2){0.0, -1.0};
                else
                    normal=(Vector2){0.0, 1.0};  // seting unity vector for normal
            }

            b->pos.x=closestX+normal.x*b->radius; 
            b->pos.y=closestY+normal.y*b->radius; //push the ball along the direction of the normal , the center of balls coordinate should be minimum one radius away from the closest edge. Because for that case, the closest point of the ball is just at the touching state with the edge. 

            //Handle bouncing off with necessary velocity

            float velocityAlongNormal = b->vel.x*normal.x+b->vel.y*normal.y; //just we took the dot product.We know normal always acts at the outer direction of the wall. So,If velocity alongnormal is negative then it means ball is moving towards the opposite direction of the normal and moving into the wall. Positive means same direction and moving away ffrom the wall. If the ball is moving along the parallel line of the wall, the this will be zero.
                                                                //If this is posotive, then it already means the ball is moving away from the wall, so it is already bounced off. so we dont need to change the velocity.
            if (velocityAlongNormal < 0.0) //We have to handle the bouncing when the that velocityalongnormal is negative. We have to push it away from the wall.
            {
                //******equation , v_new= v_old - (1+e)*(dot of v_old and normal)*normal_component*****

                b->vel.x -=(1.0+h->walls[i].bounce)* velocityAlongNormal * normal.x;  //for perfect elastic collision, when b->vel.x=10, and velocityalongnormal is -10, then simply this equation converts the velocity to -10 , it bounces off to the opposite direction with same velocity. That is exactly what we want. 
                b->vel.y -=(1.0f + h->walls[i].bounce)* velocityAlongNormal * normal.y;
            }
        }
    }
}

//Check collision with circular hole
int CheckCupCollision(Ball *b,Hole *h)
{
    float dx= b->pos.x - h->cupPos.x;
    float dy= b->pos.y - h->cupPos.y;
    float distance = sqrtf(dx*dx + dy*dy);
    float capture_dist= h->cupRadius - b->radius;

    if(capture_dist < 0.00)
    capture_dist= 0.00;

    if(Vector2Length(b->vel)<=CUP_CAPTURE_SPEED && distance<=capture_dist) return 1;
    
    return 0;
}


