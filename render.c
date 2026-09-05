#include"render.h"
#include"physics.h"
#include"raymath.h"
#include<math.h>


#define CAM_FOLLOW 6.00//how quickly the camera follows the desired position
#define LOOKAHEAD_TIME 0.35 // it means look approximately 0.35 seconds ahead in the direction the ball is moving, instead of centering the camera exactly on the ball.It predicts the balls future position. Suppose the balls velocity is 200 m per s, and this value is 0.35, so 200* 0.35=70, so the camera will predict where the ball will be after 0.35 seconds, it will be 70 meters way, the the camera moves to that position, thus it predicts the balls future position throughout the entire game.
#define LOOKAHEAD_MAX 180.00 // this prevents camera being looking too far ahead, specially when the ball is moving at a higher speed
#define AIM_MAX_LEN  460.0f //maximum length of the aiming guide
#define AIM_STEP 6.0f
#define RAIL_TOP_H 4.00
#define RAIL_BOT_H 5.00


//defining color  codes
#define COURSE_BG (Color){ 92,92,56,255}
#define GRASS_BASE (Color){ 63,163,77,255 }
#define RAIL_BASE (Color){ 192,138,69,255 }
#define SAND_BASE (Color){185,185,185,255}
#define WATER_BASE (Color){86,148,196,255}
#define ICE_BASE  (Color){198,226,240,255}
#define MUD_BASE  (Color){110,84,56,255}
#define BALL_WHITE (Color){250,250,252,255}
#define SHADOW  (Color){0,0,0,55}


//*************Camera SetUp*************//
void RenderInit(RenderState *r,Vector2 ballpos)
{
    r->cam.offset=(Vector2){GetScreenWidth()*0.5 , GetScreenHeight()*0.5}; // these two are predefined in raylib.it defines where the point should be in screen map,it is calculated as the point(target point) should be at the middle
    r->cam.target=ballpos; // At which point of the world map the camera is looking at, world map can be larger than our screen, so we have to think for both.
    r->cam.rotation=0.00;
    r->cam.zoom=1.00;
    r->hole_time=0.00; // initially the value is zero. bcz no time has been passed since the hole started.
}


//The pupose is not to show the camera outside the golf course
void ClampCamera(Camera2D *cam,Rectangle bounds) // bounds means the boundaries of the golf course inside which we want to enclose the camera
{
    float halfW=(GetScreenWidth()*0.5)/cam->zoom;// if width is 1200, then camera sees 640 pixel to the left and right of its target. Suppose if the target is at (1000,600) then cameras left position will be 1000-640, right will be 1000+640. But this creates a problem, left may become negative, that means camera will try to show outside the golf course
    float halfH=(GetScreenHeight()*0.5)/cam->zoom; 
                                                      // so min and max cases are needed  
                                                      //halfW- width of the camera
                                                      //halfH-Height of the camera
    
    float minX=bounds.x+halfW;
    float maxX=bounds.x+bounds.width-halfW; //these are the minimum and maximum values of cam->target.x . If we consider the threshold case, the cameras left edge should be aligned with the left of golf course. So, the target.x should be at least a distance of "halfW" from the bounds.x(left coordinate of the golf course)
    float minY=bounds.y+halfH;
    float maxY=bounds.y+bounds.height-halfH;
    //But when the golf course is not wider in comparison with the screen. screen width-1200, golf course=800 width, then assuming bounds.x to be zero, minX>maxX, that's the problem so we need to fix it.

    if(minX>maxX) cam->target.x=bounds.x+bounds.width*0.5; //keeping the target at the center of the course
    else cam->target.x=Clamp(cam->target.x,minX,maxX); // predefined function in "raymath" - if(value<min) return min, else if (value>max) return max, else return value

    if(minY>maxY) cam->target.y=bounds.y+bounds.height*0.5;
    else cam->target.y=Clamp(cam->target.y,minY,maxY);
}



void RenderUpdateCamera(RenderState *r,const Ball *b,const Hole *h,const Putter *p,float dt)
{
    r->hole_time+=dt; // the existing time of a hole is being increased per frame

    Vector2 desired; // where we want to the camera to move
    if(b->state==BALL_AIM || b->state==BALL_CHARGE)
    {
        Vector2 lead={cosf(p->aim_angle)*90.0,sinf(p->aim_angle)*90.0}; //cos,sin values are between -1 to 1. But we want a direction where the camera will move, and threoughout the game we are trying to keep the camera ahead of the ball without directly jumping it on the ball.
        desired=Vector2Add(b->pos,lead); //if the angle is 0 then value is(1*90,0), so the camera will 90 pixels away along the horizontal direction from the balls initial x position, and y will be similar.
    }
    else
    {
        Vector2 lead=Vector2Scale(b->vel,LOOKAHEAD_TIME); // in order to keep the camera ahead of the ball
        if(Vector2Length(lead)>LOOKAHEAD_MAX)
        lead=Vector2Scale(Vector2Normalize(lead),LOOKAHEAD_MAX); // we don't want the camera to look at a very high distance from ball(180), so keeping the direction same, we are just limiting its value.

        desired=Vector2Add(b->pos,lead);
    }
    //float k=1.00-expf(-CAM_FOLLOW*dt);
    r->cam.target=Vector2Lerp(r->cam.target,desired,0.1); //The camera will not directly jump on the target/desired position. Vector2Lerp(a,b,t)----- a=a+(b-a)*t , that means camera will move gradually to its desired position

    // camera zooming scenario will be implemented later if possible

    r->cam.offset=(Vector2){GetScreenWidth()*0.5,GetScreenHeight()*0.5};
    ClampCamera(&r->cam,h->bounds);

}


//*********World Drawing*********** //
Color SurfaceColor(SurfaceType s) // Takes surfacetype as input and returns equivalent color
{
    switch (s) {
    case SURF_FAIRWAY: return ColorBrightness(GRASS_BASE,-0.22f); // brightness factor -1 to 1, when itbis minus, the color becomes darker
    case SURF_SAND:    return SAND_BASE;
    case SURF_ICE:     return ICE_BASE;
    case SURF_MUD:     return MUD_BASE;
    case SURF_WATER:   return WATER_BASE;
    case SURF_BOOST:   return (Color){ 240, 200, 70, 255 };
    default:           return GRASS_BASE;
    }
}

void DrawCourse(const Hole *h)
{
    DrawRectangleRec(h->bounds,GRASS_BASE); //draw the gaming course(playable region)

    //drawing specks or small squares inside the course
    SetRandomSeed(h->number*7919);//This initializes thr random number generator function. And when the parameter inside this function varies then the generated random number also changes. h->number defined "which number of hole this is", so for each holes we will get different random values
    int specks=(int)((h->bounds.width*h->bounds.height)/1600.00);//total number of specks
    Color dark=ColorBrightness(GRASS_BASE,-0.13); //defining a dark color for those specks
    for(int i=0;i<specks;i++)
    {
        DrawRectangle(((int)GetRandomValue(h->bounds.x , h->bounds.x+h->bounds.width)) , ((int)GetRandomValue(h->bounds.y , h->bounds.y+h->bounds.height)) ,2 ,2,dark);

    }

    //Draw surface zone
    for(int i=0;i<h->zone_count;i++)
    {
        DrawRectangleRec(h->zones[i].area , SurfaceColor(h->zones[i].type));
    }

    //Drawing rectangular rounded dropzone {Rectangle,roundness,segments(how smoothly the corners are drawn),color}
    if(h->drop_zone.width>0.00)
    {
        for(int i=0;i<3;i++) // dividing the dropzone in three sections
        {
            DrawRectangleRounded(
                (Rectangle){ h->drop_zone.x + i * (h->drop_zone.width / 3.0f) + 8, // each section's width will be one third of the total width, and +8 is done to seperate the first section by "8 pixels" from the boundary of the dropzone
                             h->drop_zone.y + 10, //keeping the section 10 unit below from its upper boundary
                             h->drop_zone.width / 3.0f - 20, //here "-20" acts like a seperator between two consecutive sections of the dropzone
                             h->drop_zone.height - 20 }, 
                0.3f, 6, (Color){ 200, 200, 200, 235 });
        }
        DrawText("DROP", (int)(h->drop_zone.x + 30),(int)(h->drop_zone.y + h->drop_zone.height + 6), 20, BLACK); //We want to draw the txt below the dropzone section         
    }

    //Drawing cup
    DrawCircleV(h->cup_pos, h->cup_radius + 2.0f,ColorBrightness(GRASS_BASE, -0.30f)); //drawing outer rim around the main hole with a radius of 2 pixel larger
    DrawCircleV(h->cup_pos, h->cup_radius, (Color){ 18, 18, 20, 255 }); //main hole
    DrawCircleV((Vector2){ h->cup_pos.x, h->cup_pos.y + 2.0f },h->cup_radius * 0.72f, (Color){ 34, 34, 38, 255 }); //This is comparatively a smaller circle drawn slightly below the main hole(h->cup_pos+2) in order create a depth effect of the circle. Hence the hole appears to go downward
    //Three circles = outer rim + main hole + holes depth effect     
}

//Drawing the walls or rails: shadow(depth effect) + bottom dark base + main body + top light strip
void DrawRail(Rectangle r)
{
    Color base=RAIL_BASE;
    Color dark=ColorBrightness(base,-0.38f);
    Color light=ColorBrightness(base,0.30f);

    DrawRectangleRec((Rectangle){r.x+3,r.y+4,r.width,r.height},SHADOW); //Creating the depth (3D) effect
    DrawRectangleRec(r, dark); // At first drawing the whole rail by dark color
    DrawRectangleRec((Rectangle){r.x,r.y,r.width,r.height - RAIL_BOT_H },base); //Overlapping the upper portion of the rail(except the bottom base) by original rail color.Thus only the upper entire portion becomes original color, and the bottom becomes our dark base

    if (r.height > RAIL_TOP_H + RAIL_BOT_H) //Checking whether the rail can cover both "top light strip" and the "bottom dark base"
        DrawRectangleRec((Rectangle){r.x,r.y,r.width,RAIL_TOP_H},light); 
}

void DrawRails(const Hole *h)
{
    for (int i=0;i<h->wall_count;i++) DrawRail(h->walls[i].rect);
}

//Drawing the ball
void DrawBall(const Ball *b)
{
    if (b->state==BALL_OOB) return;
    DrawEllipse((int)(b->pos.x + 2),(int)(b->pos.y + 3),b->radius*1.05f,b->radius*0.55f,SHADOW);// draw the shadow effect. Ellipse-{centerX,centerY,horizontal radius,vertical radius,color}
    DrawCircleV(b->pos,b->radius,BALL_WHITE); //main ball
    DrawCircleLinesV(b->pos, b->radius, Fade(BLACK, 0.22f)); //drawing the outline of the ball by black with 22% opacity
}


// Drawing aim guide, it never creates a new ball, rather it predicts the path of the ball, then draw dashed line in the predicted path
void DrawAimGuide(const Ball *b, const Hole *h, float angle, float power)
{
    Vector2 pos=b->pos;
    Vector2 dir={cosf(angle),sinf(angle)}; // This is basically a unit vector, along which the ball is going to travel

    float len=AIM_MAX_LEN*(0.35+0.65*power); //if power=1(max), then the len will be equal to max_len , this aim guide increases with the increment of power
    Color col=Fade(WHITE,0.55);

    for (float d=0.00;d<len;d+=21.0) //we are taking 21 pixels as one portion(generally 12 pixel dash + 9 pixel gap, again 12 pixel dash and 9 pixel gap)
    {
        DrawLineEx(       //draws a line mainly - (starting point,ending point, thickness,color)
            Vector2Add(pos,Vector2Scale(dir, d)), // Its the starting position for each portion of the entire aim guide
            Vector2Add(pos,Vector2Scale(dir,fminf(d+12.0,len))),2.5,col); //fminf(d+12,len) returns the minimum between these two values. If len =105, for the 1st portion, d+12=12,len=105, it returns the endpoint=12. But for some case, it may be: len=100, but d+12 is 102, so it will return the endpoint 100.
    }
}


//Drawing the power bar
static void DrawPowerBar(float x,float y,float w,float h,float power) // Here power is significant that marks how much space will be filled up. If power=0.3, then 30% of the power bar will be filled up
{
    DrawRectangleRounded((Rectangle){x-3,y-3,w+6,h+6},0.5,8,Fade(BLACK,0.55)); // (rectangle,roundness,smoothness,color), this just draws a shaded rectangle like shadow around the actual bar.     
    Color fill=ColorLerp((Color){90,200,100,255},(Color){225,70,60,255},power); // Returns a color combining both. if power=0, returns color1(likely blue) , if power=1 ,it returns color2(likely red)
                           
    DrawRectangleRounded((Rectangle){x,y,w*power,h},0.5,8,fill); //this draws rounded rectangles inside the original bar. Width depends on power, so when power is less, width is also small, and with the increasing value of power, the color changes from green to red.
    DrawRectangleRoundedLines((Rectangle){x,y,w,h},0.5f,8,Fade(RAYWHITE,0.85)); //Drawing the outline
}


void DrawHUD(const Hole *h, const Ball *b, const Putter *p,int hole_index, int hole_count, int total)
{
    int sw=GetScreenWidth(),sh=GetScreenHeight();

    DrawRectangleRounded((Rectangle){12,12,210,78},0.2,8,Fade(BLACK, 0.45));//Draw rectangle at the top of the bar to show game huds
    DrawText(TextFormat("HOLE %d / %d",h->number,hole_count),26,20,20,RAYWHITE); //TextFormat-Formatted string
    DrawText(TextFormat("PAR %d", h->par), 26, 44, 16, Fade(RAYWHITE, 0.75));
    DrawText(TextFormat("TOTAL %d", total + b->strokes), 26, 66, 16,Fade(RAYWHITE, 0.6)); //total means counted strokes upto previous holes, and b->strokes means strokes for the current hole
    //This portion will be shown inside a rectangle, its to show the player which hole they are playing,par of that hole and their total strokes.

    const char *s=TextFormat("%d",b->strokes);
    DrawText(s,sw-30-MeasureText(s, 42),18,42,RAYWHITE); //To determine the value of x coordinate of its left , we used measuretext(text,font size)- it returns the width of the text in pixel . The purpose of this is to keep the text's right portion at least 30 pixel away from the right edge of the screen.
    DrawText("STROKES",sw - 30 - MeasureText("STROKES", 14),62,14,Fade(RAYWHITE, 0.7f)); 
    //This part is shown on the right corner of the screen for showing the strokes of the current hole.
            
    if (PutterIsCharging(p))
        DrawPowerBar(sw*0.5-130.0 , sh-54.0 , 260.0 , 22.0 , p->power); //draw the power bar while it is charging

    DrawText("LMB hold to charge, release to hit  |  RMB cancel  | wheel zoom  |  F1 editor  |  R reset",20, sh - 26, 15, Fade(RAYWHITE, 0.6f));
}

