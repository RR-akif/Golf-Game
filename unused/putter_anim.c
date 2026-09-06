
#include "putter.h"
#include "ball.h"


#include "raymath.h"
#include <math.h>




#define CHARGE_TIME 1.15f
#define BACKSWING_MAX 1.05f
#define FOLLOW_MAX 0.85f
#define STRIKE_TIME 0.09f
#define FOLLOW_TIME 0.18f
#define RECOVER_TIME 0.25f
#define CONTACT_AT 0.55f
#define MIN_POWER 0.05f
#define PUTTER_LEN 58.0f
#define HEAD_W 22.0f
#define HEAD_H 8.0f


float EaseinQuad(float t){
    return t * t;
}
float EaseOutQuad(float t){
    float u = 1.0f - t;
    return 1.0f - u * u;
}

float SmoothStep(float t){
    return t * t * (3.0f - 2.0f *t);
}
// intializing everything to zero;
void putter_init(Putter *p){
    p->phase = putter_idle;
    p->t = p->power = p->launch_power = p->aim_angle = p->offset = 0.0f;
    p->fired = false;
}

bool PutterIsCharging(const Putter *p){
    return p->phase == putter_backswing;
}

float PutterPower(const Putter *p){
    return p->power;
}

void putter_update(Putter *p, const InputState *in, Ball *ball, float dt){
    p->t += dt;
    p->aim_angle = in->aim_angle;

    bool addressable = ball->state == BALL_AIM;

    if(p->phase == putter_idle && addressable && in->charge_pressed){
        p->phase = putter_backswing;
        p->power = 0.0f;
        p->t = 0.0f; 
        p->fired = false;
    }
   switch (p->phase)
    {
        case putter_idle:
            /* ease back to zero. The 1 - expf(-k*dt) form is the
               frame-rate-independent replacement for a fixed lerp amount. */
            p->offset = Lerp(p->offset, 0.0f, 1.0f - expf(-10.0f * dt));
            break;
 
        case putter_backswing:
            if (in->charge_held) {
                p->power += dt / CHARGE_TIME;
                if (p->power > 1.0f) p->power = 1.0f;
            }
            /* the club's pull-back angle is a direct function of power, so
               the player can read their power from the club itself */
            p->offset = -BACKSWING_MAX * EaseOutQuad(p->power);
 
            if (in->cancel) {
                /* no launch, no stroke -- the ball is never touched */
                p->power       = 0.0f;
                p->launch_power = 0.0f;
                p->phase       = putter_recover;
                p->t           = 0.0f;
            }
            else if (in->charge_released) {
                if (p->power < MIN_POWER) {
                    p->power       = 0.0f;
                    p->launch_power = 0.0f;
                    p->phase       = putter_recover;
                    p->t           = 0.0f;
                } else {
                    p->launch_power = p->power;
                    p->phase       = putter_strike;
                    p->t           = 0.0f;
                }
            }
            break;
 
        case putter_strike: {
            float t    = Clamp(p->t / STRIKE_TIME, 0.0f, 1.0f);
            float back = -BACKSWING_MAX * EaseOutQuad(p->launch_power);
            p->offset  = Lerp(back, FOLLOW_MAX, EaseinQuad(t));
 
            /* fire exactly once, at the moment the club reaches the ball */
            if (!p->fired && t >= CONTACT_AT) {
                p->fired = true;
                BallLaunch(ball, DirFromAngle(p->aim_angle), p->launch_power);
            }
            if (t >= 1.0f) { p->phase = putter_follow; p->t = 0.0f; }
            break;
        }
 
        case putter_follow: {
            float t = Clamp(p->t / FOLLOW_TIME, 0.0f, 1.0f);
            p->offset = Lerp(FOLLOW_MAX, FOLLOW_MAX * 0.75f, SmoothStep(t));
            if (t >= 1.0f) { p->phase = putter_recover; p->t = 0.0f; }
            break;
        }
 
        case putter_recover: {
            float t    = Clamp(p->t / RECOVER_TIME, 0.0f, 1.0f);
            float from = (p->launch_power > 0.0f) ? FOLLOW_MAX * 0.75f
                                                 : p->offset;
            p->offset = Lerp(from, 0.0f, EaseOutQuad(t));
            if (t >= 1.0f) {
                p->phase       = putter_idle;
                p->fired       = false;
                p->power       = 0.0f;
                p->launch_power = 0.0f;
            }
            break;
        }
    }
}
 
/* ---- drawing ------------------------------------------------------------
   Procedural: rectangles and a line, no texture needed. Swap the body of
   DrawPutterShape for a single DrawTexturePro when you have art; nothing
   else changes.
   ---------------------------------------------------------------------- */
 
static void DrawPutterShape(Vector2 pivot, float angle)
{
    Vector2 dir  = { cosf(angle), sinf(angle) };
    Vector2 head = Vector2Add(pivot, Vector2Scale(dir, PUTTER_LEN));
    Vector2 sOff = { 2.0f, 3.0f };
 
    /* shadow first: later draws cover earlier ones */
    DrawLineEx(Vector2Add(pivot, sOff), Vector2Add(head, sOff), 5.0f,
               (Color){ 0, 0, 0, 60 });
 
    /* shaft */
    DrawLineEx(pivot, head, 4.0f, (Color){ 205, 208, 214, 255 });
 
    /* head: a rotated rectangle. The +90 turns it ACROSS the shaft, which
       is how a real club face sits. */
    DrawRectanglePro((Rectangle){ head.x, head.y, HEAD_W, HEAD_H },
                     (Vector2){ HEAD_W * 0.5f, HEAD_H * 0.5f },
                     angle * RAD2DEG + 90.0f, (Color){ 90, 94, 104, 255 });
 
    /* a highlight strip sells the metal */
    DrawRectanglePro((Rectangle){ head.x, head.y, HEAD_W * 0.9f, 2.0f },
                     (Vector2){ HEAD_W * 0.45f, 3.0f },
                     angle * RAD2DEG + 90.0f, (Color){ 200, 204, 212, 200 });
 
    /* grip */
    DrawCircleV(pivot, 4.5f, (Color){ 40, 42, 48, 255 });
}
 
void putter_draw(const Putter *p, const Ball *ball)
{
    if (ball->state == BALL_SUNK) return;
 
    /* hide the club while the ball is rolling, once it has settled to idle */
    if (p->phase == putter_idle && fabsf(p->offset) < 0.01f &&
        ball->state != BALL_AIM) return;
 
    float   angle = p->aim_angle + p->offset;
    Vector2 back  = { cosf(p->aim_angle), sinf(p->aim_angle) };
 
    /* the pivot sits one club length BEHIND the ball along the aim
       direction, so at zero offset the head lands exactly on the ball --
       the geometry and the animation agree with no tuning */
    Vector2 pivot = Vector2Subtract(ball->pos, Vector2Scale(back, PUTTER_LEN));
 
    DrawPutterShape(pivot, angle);
}












