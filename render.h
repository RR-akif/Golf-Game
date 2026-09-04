#ifndef RENDER_H
#define RENDER_H

#include "types.h"
#include"level.h"
#include"putter.h"
#include"input.h"

typedef struct 
{
    Camera2D cam;
    float holeTime; //How much time has passed since the hole started, and it is needed to detrmine how much time a player takes to finish a hole
} RenderState;

void RenderInit(RenderState *r, Vector2 ballPos);
void RenderUpdateCamera(RenderState *r, const Ball *b, const Hole *h,const Putter *p, float zoomDelta, float dt);
                        
void DrawCourse(const Hole *h);
void DrawBall(const Ball *b);
void DrawRails(const Hole *h);
void DrawAimGuide(const Ball *b, const Hole *h, float angle, float power);
void DrawHUD(const Hole *h, const Ball *b, const Putter *p,int holeIndex, int holeCount, int total);
             
void DrawHoleBanner(const Hole *h, float t);
void DrawScorecard(const Course *c, const int *scores);
void DrawEditorOverlay(const Editor *e, const Hole *h, Vector2 cursor);
void DrawEditorHUD(const Editor *e, const Hole *h);

#endif
