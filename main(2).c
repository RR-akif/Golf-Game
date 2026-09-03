//INCLUDE
#include "raylib.h"
#include<math.h>

//DEFINE
#define screen_w 1200
#define screen_h 800

#define ball_radius 10.00
#define hole_radius 8.00

#define friction 0.1
#define rest_speed 100
#define max_power 12000
#define hole_capture_speed 400

#define wall_x 150
#define wall_y 100
#define wall_width 900
#define wall_height 600
#define wall_restitution 0.8

//struct and enum
typedef enum{ball_resting,ball_rolling,ball_holed}Ballstate;
typedef enum{phase_aiming,phase_charging,phase_watching,phase_holed}Phase;

typedef struct
{
    float x,y;
    float velx,vely;
    float radius;
    Ballstate state;
}Ball;

typedef struct 
{
    float value;
    int direction;
    bool charging;
}Powermeter;

typedef struct 
{
    float teeX,teeY;
    float holeX,holeY;
    int par;
}Course;


//VARIABLE ASSIGNING
Ball ball;
Powermeter power;
Course course;
Phase phase;
int strokecount;
float aimangle;


//FUNCTIONS

void aimdirection(float aimangle, float *outx, float *outy)
{
    *outx=cosf(aimangle);
    *outy=sinf(aimangle);
}

float speed(const Ball *b)
{
    return sqrtf(b->velx * b->velx  + b->vely * b->vely);
}

void resethole(void)
{
    course.teeX=250.0;
    course.teeY=600.0;
    course.holeX=700.0;
    course.holeY=300.0;
    course.par=3;

    ball.x=course.teeX;
    ball.y=course.teeY;
    ball.velx=0.0;
    ball.vely=0.0;
    ball.state=ball_resting;
    ball.radius=ball_radius;

    power.value=0.0;
    power.direction=1;
    power.charging=false;

    aimangle=50.0;
    strokecount=0;
    phase=phase_aiming;
}

void updateballrolling(Ball *b , float dt)
{
    float decay=powf(friction,dt);
    b->velx *=decay;
    b->vely *=decay;
    b->x+=b->velx *dt;
    b->y+=b->vely *dt;
    
    if(speed(b)<rest_speed)
    {
        b->velx=0;
        b->vely=0;
        b->state=ball_resting;
    }
}

void checkwalls(Ball *b)
{
    if(b->x - b->radius < wall_x)
    {
        b->x = wall_x + b->radius;
        b->velx = -b->velx * wall_restitution;
    }

    if(b->x + b->radius > wall_x + wall_width)
    {
        b->x = wall_x + wall_width - b->radius;
        b->velx = -b->velx * wall_restitution;
    }
    
    if(b->y - b->radius < wall_y)
    {
        b->y = wall_y + b->radius;
        b->vely = -b->vely * wall_restitution;
    }
    
    if(b->y + b->radius > wall_y + wall_height)
    {
        b->y = wall_y + wall_height - b->radius;
        b->vely = -b->vely * wall_restitution;
    }
    
}


void checkhole(void)
{
    if(ball.state ==  ball_holed)
             return ;

    float dx = ball.x - course.holeX;
    float dy = ball.y - course.holeY;
    float dist = sqrtf(dx*dx + dy*dy);
    if(dist < hole_radius  && speed(&ball)< hole_capture_speed)
    {
        ball.state=ball_holed;
        ball.velx=0.0;
        ball.vely=0.0;
        ball.x=course.holeX;
        ball.y=course.holeY;
    }
}

void updateball(Ball *b, float dt)
{
    if(b->state != ball_rolling)
        return;

    updateballrolling(b,dt);
    checkwalls(b);
    checkhole();
}

void updatepowermeter(Powermeter *pm,float dt)
{
    if(!pm->charging)
       return;

    pm->value += pm->direction * 1.3 * dt;
    if(pm->value >= 1.0)
    {
        pm->value=1.0;
        pm->direction=-1;
    }
    if(pm->value <= 0.0)
    {
        pm->value=0.0;
        pm->direction=1;
    }
}

float getaiminput(void)
{
    float k=0.0;
    if(IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
    {
        k-=1.0;
    }
    if(IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
    {
        k+=1.0;
    }
    return k;
}


void updategame(float dt)
{
    switch(phase)
    {
        case phase_aiming:
            aimangle+= getaiminput() * 1.6 *dt;
            if(IsKeyPressed(KEY_SPACE))
            {
                power.value=0.0;
                power.direction=1.0;
                power.charging=true;
                phase=phase_charging;
            }
            break;

        case phase_charging:
            updatepowermeter(&power,dt);
            if(IsKeyReleased(KEY_SPACE))
            {
                float dirX,dirY;
                aimdirection(aimangle,&dirX,&dirY);
                float speed=power.value * max_power;
                ball.velx=speed*dirX;
                ball.vely=speed*dirY;
                ball.state=ball_rolling;

                strokecount++;
                power.charging=false;
                phase=phase_watching;
            }
            break;

        case phase_watching:
            updateball(&ball,dt);
            if(ball.state == ball_holed)
            {
                phase=phase_holed;
            }
            else if(ball.state == ball_resting)
            {
                phase=phase_aiming;
            }
            break;

        case phase_holed:
            if(IsKeyPressed(KEY_R))
            {
                resethole();
            }
            break;
    }
}


void Drawcourse(void)
{
    ClearBackground((Color){60,110,58,255});

    //Wall
    DrawRectangle(wall_x,wall_y,wall_width,wall_height,(Color){0,0,140,200});
    DrawRectangleLines(wall_x,wall_y,wall_width,wall_height,BLACK);

    //The Green around the hole
    DrawCircle(course.holeX,course.holeY,hole_radius*6.0,(Color){108,180,96,255});

    //The hole itself
    DrawCircle(course.holeX,course.holeY,hole_radius*3.0,(Color){18,18,18,255});

    //Flag
    DrawLine(course.holeX,course.holeY,course.holeX,course.holeY-40,RAYWHITE);
    DrawTriangle((Vector2){course.holeX,course.holeY-40},(Vector2){course.holeX,course.holeY-20},(Vector2){course.holeX+10,course.holeY-30},RED);

    //Tee
    DrawCircle(course.teeX,course.teeY,10,Fade(RAYWHITE,0.5f));
}


void Drawaim(void)
{
    float dirX,dirY;
    aimdirection(aimangle, &dirX, &dirY);


    DrawLine(ball.x,ball.y,ball.x+dirX*0.5 , ball.y+dirY*0.5 ,YELLOW );

    if(phase == phase_charging)
    {
        float px=ball.x;
        float py=ball.y;
        float vx=power.value* dirX * max_power;
        float vy=power.value * dirY * max_power;
        float dt=GetFrameTime();

        for(int i=0;i<300;i++)
        {
            float decay=powf(friction,dt);
            vx*=decay;
            vy*=decay;
            px+= vx*dt;
            py+= vy*dt;
            if(sqrtf(vx*vx + vy*vy) < rest_speed)
            break;
            if(i%6==0)
            {
                DrawCircle(px,py,2,Fade(RAYWHITE,0.45f));
            }
        }
    }
}


//Stroke Visualize
const char *Scorename(int strokes,int par)//pointer to a const character
{
    if(strokes==1)
        return "Hole in One";

    int diff=strokes-par;
    switch(diff)
    {
        case -2:return "Eagle";
        case -1:return "Birdie";
        case 0:return "par";
        case 1: return "Bogey";
        case 2:return "Double Bogey";
        default:return (diff<0)?"Great":"Over par";
    }
}


void Drawgame(void)
{
    BeginDrawing();
    Drawcourse();
    if(phase == phase_aiming || phase== phase_charging)
    {
        Drawaim();
    }

    DrawCircle(ball.x,ball.y,ball_radius,RAYWHITE);

    //HUD
    DrawText(TextFormat("Stroke: %d   Par: %d",strokecount,course.par),20,20,24,BLACK);
    float dx=ball.x-course.holeX ;
    float dy = ball.y - course.holeY;
    float dist=sqrtf(dx*dx + dy*dy);
    DrawText(TextFormat("Distance: %.1f m",dist),20,52,20,BLACK);

    if(phase == phase_charging)
    {
        int x=30;
        int y=screen_h - 60;
        int w= 240;
        int h=24;
        DrawRectangle(x,y,w,h,Fade(BLACK,0.5f));
        DrawRectangle(x,y,(int)(w*power.value),h,ColorLerp(GREEN,RED,power.value));
        DrawRectangleLines(x,y,w,h,RAYWHITE);
    }

    if(phase == phase_aiming)
    {
        DrawText("Hold A/D: Aim    SPACE:  Hold to Charge,Release to Hit",20,screen_h-40,18,Fade(RAYWHITE,0.85f));
    }

    if(phase == phase_holed)
    {
        const char *msg=Scorename(strokecount,course.par);
        int tw=MeasureText(msg,48);
        DrawRectangle(0,screen_h -60, screen_w,120,Fade(BLACK,0.6f));
        DrawText(msg,screen_w/2- tw/2,screen_h/2+22 ,22,GOLD);
        const char *sub=TextFormat("%d strokes!!  Press R to replay",strokecount);
        int sw=MeasureText(sub,22);
        DrawText(sub,screen_w/2-sw/2,screen_h/2+22,22,RAYWHITE);
    }
    EndDrawing();
}



int main(void)
{
    InitWindow(screen_w, screen_h, "Golf Game");
    SetTargetFPS(60);
    resethole();
    

    while (!WindowShouldClose())
    {
        float dt=GetFrameTime();
        updategame(dt);
        Drawgame();
    }

    CloseWindow();
    return 0;
}

